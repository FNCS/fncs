/* C++ headers */
#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

/* C headers */
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/* 3rd party headers */
#include <mpi.h>

#define HOST_NAME_SIZE 256

/* bad design, global static, but it was simple and fast */
static int world_rank;

std::string parse_input_file(const char *filename);
bool check_program(const char *progpath);

class Log {
    public:
        Log(): os(), str("") {}
        virtual ~Log() {
            if (!str.empty()) {
                os << ": " << str;
            }
            std::cerr << world_rank << ": " << os.str() << std::endl;
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
    private:
        Log(const Log&);
        Log& operator =(const Log&);
};
#define ERRNO Log().Get(errno)
#define ERR Log().Get()

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

    /* basic MPI initialization, rank, and size info */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    /* error checking for input file */
    if (2 != argc) {
        if (0 == world_rank) {
            std::cerr << "Missing input file" << std::endl;
        }
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    /* parse the input file to assign commands to MPI ranks */
    command = parse_input_file(argv[1]);

    /* retrieve host name for debugging, eventual broker location */
    retval = gethostname(host_name, HOST_NAME_SIZE);
    if (0 != retval) {
        ERRNO << "gethostname";
        MPI_Abort(MPI_COMM_WORLD, -1);
    }

    /* debugging print of all assigned commands */
    for (int i=0; i<world_size; ++i) {
        if (i == world_rank) {
            std::cout << i << ": "
                << host_name << ": "
                << command << std::endl;
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
        std::cerr << world_rank
            << ": invalid command string: "
            << command << std::endl;
        MPI_Abort(MPI_COMM_WORLD, -1);
    }
    /* attempt to change path now and report any error */
    if (tokens[0] != "pwd") {
        retval = chdir(tokens[0].c_str());
        if (0 != retval) {
            ERRNO << "chdir(" << tokens[0] << ")";
            MPI_Abort(MPI_COMM_WORLD, -1);
        }
    }
    /* set any env vars from the input file */
    while (tok < tokens.size()) {
        size_t pos = tokens[tok].find('=');
        if (std::string::npos != pos) {
            std::string key = tokens[tok].substr(0, pos);
            std::string val = tokens[tok].substr(pos+1);
            setenv(key.c_str(), val.c_str(), 1);
            ++tok;
#if 0
            std::cout << world_rank << ": "
                << key << "=" << val << std::endl;
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
        MPI_Abort(MPI_COMM_WORLD, -1);
    }

    std::vector<char*> new_argv;
    for (; tok<tokens.size(); ++tok) {
        new_argv.push_back(strdup(tokens[tok].c_str()));
    }
    new_argv.push_back(NULL);

    /* debugging print of all parsed commands */
    for (int i=0; i<world_size; ++i) {
        if (i == world_rank) {
            std::cout << i << ": "
                << host_name << ": ";
            for (int j=0; NULL != new_argv[j]; ++j) {
                std::cout << new_argv[j] << " ";
            }
            std::cout << std::endl;
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
    /* We're done with MPI at this point. It is unsafe to call fork from
     * an MPI program, so we terminate MPI early. */
    MPI_Finalize();

    pid_t pid = fork();
    if (-1 == pid) {
        /* fork error */
        ERRNO << "fork";
        exit(EXIT_FAILURE);
    } else if (0 == pid) {
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
        /* parent, does nothing */
    }

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
            std::cerr << "Launcher input file missing actual data." << std::endl;
            MPI_Abort(MPI_COMM_WORLD, -1);
        }
        while (fin.good() && counter<world_size-1) {
            ++counter;
            MPI_Send(line.c_str(), line.size(), MPI_CHAR,
                    counter, 0, MPI_COMM_WORLD);
            getline(fin, line);
        }
        if (counter < world_size-1) {
            std::cerr << "Launcher input file contained "
                << counter << " lines and you requested "
                << world_size << " MPI ranks."
                << std::endl
                << "You need " << counter << " plus 1 for the broker."
                << std::endl;
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
            std::cerr << "Launcher input file contained "
                << counter << " lines and you requested "
                << world_size << " MPI ranks."
                << std::endl
                << "You need " << counter << " plus 1 for the broker."
                << std::endl;
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
        std::string program = path[0] + "/" + progpath;
        if (0 == access(program.c_str(), X_OK)) {
            return true;
        }
    }

    return false;
}

