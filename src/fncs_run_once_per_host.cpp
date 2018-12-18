#include "config.h"

/* C++ headers */
#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

/* C headers */
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

/* 3rd party headers */
#include <mpi.h>

#define HOST_NAME_SIZE 256

/* bad design, global static, but it was simple and fast */
static int world_rank;

std::vector<std::string> split(const std::string& s, char delimiter);
bool check_program(const char *progpath);

class Log {
    public:
        Log(bool is_err_=true): os(), str(""), is_err(is_err_) {}
        virtual ~Log() {
            if (!str.empty()) {
                os << ": " << str;
            }
            if (is_err) {
                std::cerr << world_rank << ": " << os.str() << std::endl;
            }
            else {
                std::cout << world_rank << ": " << os.str() << std::endl;
            }
        }
        std::ostringstream& Get() {
            return os;
        }
        std::ostringstream& Get(int errnum) {
            str = strerror(errnum);
            return os;
        }
    protected:
        std::ostringstream os;
        std::string str;
        bool is_err;
    private:
        Log(const Log&);
        Log& operator =(const Log&);
};
#define ERRNO Log().Get(errno)
#define ERR Log().Get()
#define LOG Log(false).Get()

template <typename T>
std::string to_string(T object) {
    std::ostringstream os;
    os << object;
    return os.str();
}


int main(int argc, char **argv)
{
    int world_size = 0;
    char host_name[HOST_NAME_SIZE] = {0};
    long hostid;
    std::vector<long> hostids;
    int retval = 0;

    /* basic MPI initialization, rank, and size info */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    /* retrieve host name for debugging */
    retval = gethostname(host_name, HOST_NAME_SIZE);
    if (0 != retval) {
        ERRNO << "gethostname";
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    /* retrieve hostid */
    hostid = gethostid();

    /* debugging print of all hostid */
    for (int i=0; i<world_size; ++i) {
        if (i == world_rank) {
            LOG << "hostname: " << host_name
                << "\thostid: " << hostid;
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }

    /* exchange them all */
    hostids.resize(world_size);
    hostids[world_rank] = hostid;
    MPI_Allgather(MPI_IN_PLACE, 0, MPI_DATATYPE_NULL,
            &hostids[0], 1, MPI_LONG, MPI_COMM_WORLD);

    bool is_master = true;
    /* determine which rank is the first on each node */
    for (int i=0; i<world_rank; ++i) {
        if (hostids[i] == hostid) {
            is_master = false;
            break;
        }
    }

    /* debugging print of all assigned commands */
    for (int i=0; i<world_size; ++i) {
        if (i == world_rank) {
            LOG << host_name << ": " << hostid << ": "
                << (is_master?"MASTER":"");
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }

    /* We're mostly done with MPI at this point. It is unsafe to call
     * fork from an MPI program, but we are only relying on MPI_Abort
     * beyond this point. Ideally, we terminate MPI early, right now. */
    /*MPI_Finalize();*/

    if (is_master) {
        const char *program = argv[1];
        /* check that we can execute the program */
        if (!check_program(program)) {
            ERR << program << ": No such file or directory";
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        pid_t cpid = fork();
        if (-1 == cpid) {
            /* fork error */
            ERRNO << "fork";
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        } else if (0 == cpid) {
            /* this is the child */
            int fd = open("fncs.out", O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
            if (-1 == fd) {
                ERRNO << "open(fncs.out)";
                _exit(EXIT_FAILURE);
            }
            retval = dup2(fd, 1); /* make stdout go to file */
            if (-1 == retval) {
                ERRNO << "dup2(stdout)";
                _exit(EXIT_FAILURE);
            }
            retval = dup2(fd, 2); /* make stderr go to same file */
            if (-1 == retval) {
                ERRNO << "dup2(stderr)";
                _exit(EXIT_FAILURE);
            }
            retval = close(fd); /* fd no longer needed, dup'ed handles are sufficient */
            if (0 != retval) {
                ERRNO << "close(fncs.out)";
                _exit(EXIT_FAILURE);
            }
            retval = execvp(argv[1], &argv[1]);
            if (-1 == retval) {
                ERRNO << "execvp(" << argv[1] << ", ...)";
                _exit(EXIT_FAILURE);
            }
        } else {
            /* parent */
            int status;
            pid_t w = waitpid(cpid, &status, 0);
            if (w == -1) {
                ERRNO << "waitpid";
                MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
            }
            if (WIFEXITED(status)) {
                LOG << "exited, status=" << WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
                ERR << "killed by signal " << WTERMSIG(status);
                MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
            } else if (WIFSTOPPED(status)) {
                ERR << "stopped by signal " << WSTOPSIG(status);
                MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
            }
        }
    }

    MPI_Finalize();
    return 0;
}


std::vector<std::string> split(const std::string& s, char delimiter)
{
   std::vector<std::string> tokens;
   std::string token;
   std::istringstream tokenStream(s);
   while (std::getline(tokenStream, token, delimiter)) {
      tokens.push_back(token);
   }
   return tokens;
}


/* check for progpath in PATH env var and that it is executable */
bool check_program(const char *progpath)
{
    if (progpath[0] == '/') {
        /* absolute path was given, don't check PATH */
        return 0 == access(progpath, X_OK);
    }

    std::string PATH = getenv("PATH");
    if (PATH.empty()) {
        ERR << "PATH env var was empty";
        return false;
    }

    std::vector<std::string> path = split(PATH,':');
    for (size_t i=0; i<path.size(); ++i) {
        std::string program = path[i] + "/" + progpath;
        if (0 == access(program.c_str(), X_OK)) {
            return true;
        }
    }

    return false;
}

