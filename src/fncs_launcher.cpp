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

std::string parse_input_file(const char *filename);
std::vector<std::string> split(const std::string& s);
std::vector<std::string> split(const std::string& s, char delimiter);
bool check_program(const char *progpath);
bool dir_exists(const char *path);
bool dir_empty(const char *path);

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
    std::string command;
    char host_name[HOST_NAME_SIZE] = {0};
    int retval = 0;
    size_t tok = 1;
    std::string archive_dir;

    /* basic MPI initialization, rank, and size info */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    /* error checking for input file */
    if (3 != argc) {
        if (0 == world_rank) {
            if (1 == argc) {
                ERR << "Missing input file";
            }
            if (2 == argc) {
                ERR << "Missing output directory";
            }
        }
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    /* make sure our output directory is empty so that we don't
     * overwrite any contents of a previous co-sim */
    archive_dir = argv[2];
    if (archive_dir[0] != '/') {
        if (0 == world_rank) {
            ERR << archive_dir << ": output directory must be an aboluste path";
        }
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
    if (0 == world_rank) {
        if (dir_exists(archive_dir.c_str())) {
            if (!dir_empty(archive_dir.c_str())) {
                ERR << archive_dir << " is not an empty directory";
                MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
            }
        }
        else {
            /* attempt to create directory,
             * owner and group has read/write/exec bits */
            if (-1 == mkdir(archive_dir.c_str(), S_IRWXU|S_IRWXG)) {
                ERRNO << archive_dir;
                MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
            }
        }
    }

    /* parse the input file to assign commands to MPI ranks */
    command = parse_input_file(argv[1]);

    /* retrieve host name for debugging, eventual broker location */
    retval = gethostname(host_name, HOST_NAME_SIZE);
    if (0 != retval) {
        ERRNO << "gethostname";
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    /* debugging print of all assigned commands */
    for (int i=0; i<world_size; ++i) {
        if (i == world_rank) {
            LOG << host_name << ": " << command;
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }

    /* broadcast the rank 0 host name,
     * where the broker will be launched */
    if (0 == world_rank) {
        const char *env = "tcp://*:7777";
        MPI_Bcast(host_name, HOST_NAME_SIZE, MPI_CHAR, 0, MPI_COMM_WORLD);
        setenv("FNCS_BROKER", env, 1);
    }
    if (0 != world_rank) {
        char env[HOST_NAME_SIZE] = {0};
        MPI_Bcast(host_name, HOST_NAME_SIZE, MPI_CHAR, 0, MPI_COMM_WORLD);
        strcpy(env, "tcp://");
        strcat(env, host_name);
        strcat(env, ":7777");
        setenv("FNCS_BROKER", env, 1);
    }
    setenv("FNCS_LAUNCHER_RANK", to_string(world_rank).c_str(), 1);

    /* parse the command so we can call fork exec */
    /* command is parsed based on whitespace */
    /* exec command requires NULL terminated array of char* */
    /* preserved quoted arguments */
    std::vector<std::string> tokens;
    std::istringstream iss(command);
    std::istream_iterator<std::string> it(iss);
    std::istream_iterator<std::string> eos;
    bool has_quote = false;
    while (it!=eos) {
        std::string token = *it;
        if (has_quote) {
            if (*token.rbegin() == '"') {
                has_quote = false;
            }
            tokens.back() += " " + token;
        }
        else {
            if (token[0] == '"') {
                has_quote = true;
            }
            tokens.push_back(token);
        }
        ++it;
    }

    /* there must be at least 2 tokens,
     * 1) working directory,
     * 2) program name */
    if (tokens.size() < 2) {
        ERR << "invalid command string: " << command;
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    /* attempt to change path now and report any error */
	std::string working_directory = tokens[0];
    if (working_directory == "pwd") {
        char buf[PATH_MAX];
        if (NULL == getcwd(buf, PATH_MAX)) {
            ERRNO << "getcwd";
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        working_directory = buf;
    }
    else {
        retval = chdir(working_directory.c_str());
        if (0 != retval) {
            ERRNO << "chdir(" << working_directory << ")";
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
    }
    /* figure out the zipdir early, in case of error */
    std::string zipdir;
    if (0 != world_rank) {
        std::string::size_type found = working_directory.rfind('/');
        if (found == std::string::npos) {
            ERR << working_directory << ": could not determine directory to zip";
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        zipdir = working_directory.substr(found+1);
    }
    /* set any env vars from the input file */
    tok = 1;
    while (tok < tokens.size()) {
        size_t pos = tokens[tok].find('=');
        if (std::string::npos != pos) {
            std::string key = tokens[tok].substr(0, pos);
            std::string val = tokens[tok].substr(pos+1);
            setenv(key.c_str(), val.c_str(), 1);
            ++tok;
#if 0
            LOG << key << "=" << val;
#endif
        }
        else {
            break; /* no key=value found, break out */
        }
    }

    const char *program = tokens[tok].c_str();
    /* check that we can execute the program */
    if (!check_program(program)) {
        ERR << program << ": No such file or directory";
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    std::vector<char*> new_argv;
    for (; tok<tokens.size(); ++tok) {
        new_argv.push_back(strdup(tokens[tok].c_str()));
    }
    new_argv.push_back(NULL);

    /* debugging print of all parsed commands */
    for (int i=0; i<world_size; ++i) {
        if (i == world_rank) {
            std::cout << i << ": ";
            for (int j=0; NULL != new_argv[j]; ++j) {
                std::cout << new_argv[j] << " ";
            }
            std::cout << std::endl;
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }

    /* We're mostly done with MPI at this point. It is unsafe to call
     * fork from an MPI program, but we are only relying on MPI_Abort
     * beyond this point. Ideally, we terminate MPI early, right now. */
    /*MPI_Finalize();*/

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
        retval = execvp(new_argv[0], &new_argv[0]);
        if (-1 == retval) {
            ERRNO << "execvp(" << new_argv[0] << ", ...)";
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

    /* If we got this far, we can zip up our working directory
     * and store it at the given location. We rely on an external
     * zip or zip-like tool. */
    /* we don't zip up the fncs_broker working dir */
    if (0 == world_rank) {
        for (int i=1; i<world_size; ++i) {
            MPI_Barrier(MPI_COMM_WORLD);
        }
    }
    else {
        std::string zipfile = zipdir + "." + ZIP_EXT;
        program = ZIP;
        tokens.clear();
        new_argv.clear(); /* leaks strdup'd strings */
        tokens.push_back(ZIP);
        tokens.push_back(ZIP_ARGS);
        tokens.push_back(zipfile);
        tokens.push_back(zipdir);

        tok = 0;
        for (; tok<tokens.size(); ++tok) {
            new_argv.push_back(strdup(tokens[tok].c_str()));
        }
        new_argv.push_back(NULL);

        /* debugging print of all parsed commands */
        for (int i=1; i<world_size; ++i) {
            if (i == world_rank) {
                std::cout << i << ": ";
                for (int j=0; NULL != new_argv[j]; ++j) {
                    std::cout << new_argv[j] << " ";
                }
                std::cout << std::endl;
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }

        /* working directory for archiving is up one diretory */
        retval = chdir("..");
        if (0 != retval) {
            ERRNO << "chdir(..) for archive";
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        cpid = fork();
        if (-1 == cpid) {
            /* fork error */
            ERRNO << "fork archive";
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        } else if (0 == cpid) {
            /* this is the child */
            std::string archive_out = zipdir + ".out";
            int fd = open(archive_out.c_str(), O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
            if (-1 == fd) {
                ERRNO << "open(" << archive_out << ")";
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
                ERRNO << "close(" << archive_out << ")";
                _exit(EXIT_FAILURE);
            }
            retval = execvp(new_argv[0], &new_argv[0]);
            if (-1 == retval) {
                ERRNO << "execvp(" << new_argv[0] << ", ...)";
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
                LOG << "archive exited, status=" << WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
                ERR << "archive killed by signal " << WTERMSIG(status);
                MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
            } else if (WIFSTOPPED(status)) {
                ERR << "archive stopped by signal " << WSTOPSIG(status);
                MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
            }
        }

        /* now copy the zip file to the final destination */
        {
            std::string dst_str = archive_dir + "/" + zipfile;
            std::ifstream src(zipfile.c_str(), std::ios::binary);
            std::ofstream dst(dst_str.c_str(), std::ios::binary);
            dst << src.rdbuf();
        }
    }

    MPI_Finalize();
    return 0;
}


/* Parses the given file containing our command list.
 *
 * The file has the format, one per line
 * [KEY1=VALUE1] ... [KEYN=VALUEN] program_name [arg1] ... [argn]
 */
std::string parse_input_file(const char *filename)
{
    int world_rank;
    int world_size;
    int counter = 0;
    std::string myline;

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    /* Rank 0 opens the file for parsing,
     * broadcasts the contents to all ranks. */
    if (0 == world_rank) {
        std::ifstream fin(filename);
        std::string line;
        /* input file might start with comment(s), skip them all */
        do {
            std::getline(fin, line);
        } while (fin.good() && line[0] == '#');
        if (line.empty() || line[0] == '#') {
            /* file was only comments */
            ERR << "Launcher input file missing actual data.";
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        while (fin.good() && counter<world_size-1) {
            ++counter;
            MPI_Send(line.c_str(), line.size(), MPI_CHAR,
                    counter, 0, MPI_COMM_WORLD);
            getline(fin, line);
        }
        if (counter < world_size-1) {
            ERR << "Launcher input file contained "
                << counter << " lines and you requested "
                << world_size << " MPI ranks."
                << std::endl
                << "You need " << counter << " plus 1 for the broker.";
            /* go ahead and send remaining empty messages */
            for (int i=counter+1; i<world_size; ++i) {
                MPI_Send("", 0, MPI_CHAR, i, 0, MPI_COMM_WORLD);
            }
        }
        else if (counter == world_size-1 && fin.good()) {
            while (fin.good()) {
                ++counter;
                getline(fin, line);
            }
            ERR << "Launcher input file contained "
                << counter << " lines and you requested "
                << world_size << " MPI ranks."
                << std::endl
                << "You need " << counter << " plus 1 for the broker.";
        }
        std::ostringstream os;
        //os << "pwd fncs_broker " << counter;
        os << "pwd fncs_broker " << counter;
        myline = os.str();
        MPI_Bcast(&counter, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }
    else {
        int count;
        MPI_Status status;
        MPI_Probe(0, 0, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_CHAR, &count);
        myline.assign(count+1, '\0');
        MPI_Recv(&myline[0], count, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Bcast(&counter, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }

    if (counter != world_size-1) {
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    return myline;
}


std::vector<std::string> split(const std::string& s)
{
	std::vector<std::string> tokens;
	std::istringstream iss(s);
	std::copy(std::istream_iterator<std::string>(iss),
			std::istream_iterator<std::string>(),
			std::back_inserter(tokens));
	return tokens;
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


bool dir_exists(const char *path) {
    struct stat sb;
    return stat(path, &sb) == 0 && S_ISDIR(sb.st_mode);
}


bool dir_empty(const char *path)
{
    struct dirent *ent = NULL;
    DIR *d = NULL;

    d = opendir(path);
    if (NULL == d) {
        ERRNO << path;
        return false;
    }

    bool ret = true;
    while ((ent = readdir(d))) {
        if (!strcmp(ent->d_name, ".") || !(strcmp(ent->d_name, "..")))
            continue;
        ret = false;
        break;
    }

    if (-1 == closedir(d)) {
        ERRNO << path;
    }

    return ret;
}

