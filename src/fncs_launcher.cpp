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

std::vector<std::string> parse_input_file(const char *filename);
std::vector<std::string> split(const std::string& s);
std::vector<std::string> split(const std::string& s, char delimiter);
bool check_program(const char *progpath);
bool dir_exists(const char *path);
bool dir_empty(const char *path);
void killall(std::vector<pid_t> children);
bool check_command(std::string command);
pid_t run_command(std::string command);
pid_t unzip_command(std::string command);
bool zip_command(std::string command, std::string archive_dir);

enum loglevel_e
    {logWARNING, logINFO, logDEBUG, logDEBUG1, logDEBUG2, logDEBUG3, logDEBUG4};

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

loglevel_e launcher_log_level;

// these outputs we always want!
#define ERRNO Log().Get(errno)
#define ERR Log().Get()

#define LAUNCHER_LOG(level) \
    if (level > launcher_log_level) ; \
    else Log(false).Get()

#define LWARNING LAUNCHER_LOG(logWARNING)
#define LINFO LAUNCHER_LOG(logINFO)
#define LDEBUG LAUNCHER_LOG(logDEBUG)
#define LDEBUG1 LAUNCHER_LOG(logDEBUG1)
#define LDEBUG2 LAUNCHER_LOG(logDEBUG2)
#define LDEBUG3 LAUNCHER_LOG(logDEBUG3)
#define LDEBUG4 LAUNCHER_LOG(logDEBUG4)

template <typename T>
std::string to_string(T object) {
    std::ostringstream os;
    os << object;
    return os.str();
}


int main(int argc, char **argv)
{
    int world_size = 0;
    std::vector<std::string> commands;
    char host_name[HOST_NAME_SIZE] = {0};
    int retval = 0;
    std::string archive_dir;
    std::string broker_host = "tcp://*:7777";
    std::string broker_client;

    /* start the logging feature, check enviroment*/
    char *log_level_export = NULL;
    log_level_export = getenv("FNCS_LAUNCHER_LOG_LEVEL");

    if (!log_level_export) {
        launcher_log_level = logINFO; 
    } else if (strcmp(log_level_export,"WARNING") == 0) {
        launcher_log_level = logWARNING;
    } else if (strcmp(log_level_export,"INFO") == 0) {
        launcher_log_level = logINFO;
    } else if (strcmp(log_level_export,"DEBUG") == 0) {
        launcher_log_level = logDEBUG;
    } else if (strcmp(log_level_export,"DEBUG1") == 0) {
        launcher_log_level = logDEBUG1;
    } else if (strcmp(log_level_export,"DEBUG2") == 0) {
        launcher_log_level = logDEBUG2;
    } else if (strcmp(log_level_export,"DEBUG3") == 0) {
        launcher_log_level = logDEBUG3;
    } else if (strcmp(log_level_export,"DEBUG4") == 0) {
        launcher_log_level = logDEBUG4;
    } else {
        launcher_log_level = logINFO;
        LINFO << "The FNCS_LAUNCHER_LOG_LEVEL was set to an unknown value";
    }

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
    commands = parse_input_file(argv[1]);

    /* retrieve host name for debugging, eventual broker location */
    retval = gethostname(host_name, HOST_NAME_SIZE);
    if (0 != retval) {
        ERRNO << "gethostname";
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    /* informative print of all assigned commands */
    /*
    for (int i=0; i<world_size; ++i) {
        if (i == world_rank) {
            for (size_t j=0; j<commands.size(); ++j) {
                LINFO << host_name << ": " << commands[j];
            }
        }
        // include this if you want ordered output from MPI, will slow down execution significantly
        //MPI_Barrier(MPI_COMM_WORLD);
    }
    */

    /* broadcast the rank 0 host name,
     * where the broker will be launched */
    MPI_Bcast(host_name, HOST_NAME_SIZE, MPI_CHAR, 0, MPI_COMM_WORLD);
    broker_client = "tcp://";
    broker_client += host_name;
    broker_client += ":7777";

    /* we must reset our working directory before any run of a command
     * because the command string has its own working directory */
    char cwd[PATH_MAX];
    if (NULL == getcwd(cwd, PATH_MAX)) {
        ERRNO << "getcwd";
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    /* verify we can run the commands first */
    int checksum = 0;
    for (size_t i=0; i<commands.size(); ++i) {
        int retval = chdir(cwd);
        if (0 != retval) {
            ERRNO << "chdir(" << cwd << ")";
            checksum = 1;
        }
        /*
        bool check = check_command(commands[i]);
        if (!check) {
            ERR << "Check command failed: " << commands[i];
            checksum = 1;
        }
        */
    }
    MPI_Allreduce(MPI_IN_PLACE, &checksum, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    if (0 != checksum) {
        if (0 == world_rank) {
            ERR << "Check command failed";
        }
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    /* We're mostly done with MPI at this point. It is unsafe to call
     * fork from an MPI program, but we are only relying on MPI_Abort
     * beyond this point. Ideally, we terminate MPI early, right now. */
    MPI_Finalize();

    /* for the GridLAB-D federates we need to unzip the model files we need */
    std::vector<pid_t> children;
    for (size_t i=0; i<commands.size(); ++i) {
        int retval = chdir(cwd);
        if (0 != retval) {
            ERRNO << "chdir(" << cwd << ")";
        }
        if (0 == world_rank && 0 == i) {
            continue; /* skip zipping broker command */
        }
        pid_t child = unzip_command(commands[i]); 
        if (-1 == child) {
            ERRNO << "unzip_command(" << commands[i] << ")";
            killall(children);
        }
        children.push_back(child);
    }
    
    /* wait for all children */
    if (!commands.empty()) {
        int status;
        pid_t pid;
        std::string command_exit;
        while ((pid = wait(&status)) > 0) {
            /* parent */
            if (pid == -1) {
                ERRNO << "wait";
                exit(EXIT_FAILURE);
            }
            if (WIFEXITED(status)) { 
                LDEBUG << "unzip exited, status=" << WEXITSTATUS(status);
                if (0 != WEXITSTATUS(status)) {
                    exit(EXIT_FAILURE);
                }
            } else if (WIFSIGNALED(status)) {
                for (std::vector<pid_t>::size_type i = 0; i != children.size(); i++) {
                    if (pid == children[i]) {
                        command_exit = commands[i];
                    }       
                }
                ERR << "unzip killed by signal " << WTERMSIG(status) << " (" << command_exit << ")";
                exit(EXIT_FAILURE);
            } else if (WIFSTOPPED(status)) {
                for (std::vector<pid_t>::size_type i = 0; i != children.size(); i++) {
                    if (pid == children[i]) {
                        command_exit = commands[i];
                    }       
                }
                ERR << "unzip stopped by signal " << WSTOPSIG(status) << " (" << command_exit << ")";
                exit(EXIT_FAILURE);
            }
        }
    }
    
    /* time to run the actual commands */
    children.clear();
    bool any_error = false;
    for (size_t i=0; i<commands.size(); ++i) {
        int retval = chdir(cwd);
        if (0 != retval) {
            ERRNO << "chdir(" << cwd << ")";
            killall(children);
        }
        /* first command of rank 0 is always broker */
        if (0 == world_rank && 0 == i) {
            setenv("FNCS_BROKER", broker_host.c_str(), 1);
        }
        else {
            setenv("FNCS_BROKER", broker_client.c_str(), 1);
        }
        /* this env var is used by a special test program only */
        {
            std::string val = to_string(world_rank);
            val += "_";
            val += to_string(i);
            setenv("FNCS_LAUNCHER_RANK", val.c_str(), 1);
            LDEBUG << "FNCS_LAUNCHER_RANK=" << val;
        }
        pid_t child = run_command(commands[i]);
        if (-1 == child) {
            ERRNO << "run_command(" << commands[i] << ")";
            killall(children);
        }
        // some initial information about where each process is started
        LINFO << host_name << ", " << child << ": " << commands[i];
        children.push_back(child);
    }

    /* wait for all children */
    if (!commands.empty()) {
        int status;
        pid_t pid;
        std::string command_exit;
        while ((pid = wait(&status)) > 0) {
            /* parent */
            if (pid == -1) {
                ERRNO << "wait";
                any_error |= true;
            }
            if (WIFEXITED(status)) { 
                LDEBUG << "child exited, status=" << WEXITSTATUS(status);
                any_error |= (0 != WEXITSTATUS(status));
                if (WEXITSTATUS(status) != 0) {
                    for (std::vector<pid_t>::size_type i = 0; i != children.size(); i++) {
                        if (pid == children[i]) {
                            command_exit = commands[i];
                        }       
                    }
                    ERR << "child exited with non-zero status, " << WEXITSTATUS(status) << " (" << command_exit << ")";
                }
            } else if (WIFSIGNALED(status)) {
                for (std::vector<pid_t>::size_type i = 0; i != children.size(); i++) {
                    if (pid == children[i]) {
                        command_exit = commands[i];
                    }       
                }
                ERR << "killed by signal " << WTERMSIG(status) << " (" << command_exit << ")";
                any_error |= true;
            } else if (WIFSTOPPED(status)) {
                for (std::vector<pid_t>::size_type i = 0; i != children.size(); i++) {
                    if (pid == children[i]) {
                        command_exit = commands[i];
                    }       
                }
                ERR << "stopped by signal " << WSTOPSIG(status) << " (" << command_exit << ")";
                any_error |= true;
            }
        }

        /* regardless of errors, we zip all working directories */
        for (size_t i=0; i<commands.size(); ++i) {
            int retval = chdir(cwd);
            if (0 != retval) {
                ERRNO << "chdir(" << cwd << ")";
            }
            if (0 == world_rank && 0 == i) {
                continue; /* skip zipping broker command */
            }
            any_error |= zip_command(commands[i], archive_dir);
        }
    }

    if (0 == world_rank) {
        int retval = chdir(cwd);
        if (0 != retval) {
            ERRNO << "chdir(" << cwd << ")";
        }
        std::string dst_str = archive_dir + "/broker.out";
        std::ifstream src("fncs.out", std::ios::binary);
        std::ofstream dst(dst_str.c_str(), std::ios::binary);
        dst << src.rdbuf();
    }

    return any_error ? EXIT_FAILURE : EXIT_SUCCESS;
}


void killall(std::vector<pid_t> children)
{
    for (size_t i=0; i<children.size(); ++i) {
        kill(children[i], SIGKILL);
    }
    exit(EXIT_FAILURE);
}


bool check_command(std::string command)
{
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
        return false;
    }
    /* attempt to change path now and report any error */
	std::string working_directory = tokens[0];
    if (working_directory == "pwd") {
        char buf[PATH_MAX];
        if (NULL == getcwd(buf, PATH_MAX)) {
            ERRNO << "getcwd";
            return false;
        }
        working_directory = buf;
    }
    else {
        int retval = chdir(working_directory.c_str());
        if (0 != retval) {
            ERRNO << "chdir(" << working_directory << ")";
            return false;
        }
    }
    /* figure out the zipdir early, in case of error */
    std::string zipdir;
    if (0 != world_rank) {
        std::string::size_type found = working_directory.rfind('/');
        if (found == std::string::npos) {
            ERR << working_directory << ": could not determine directory to zip";
            return false;
        }
        zipdir = working_directory.substr(found+1);
    }
    /* set any env vars from the input file */
    size_t tok = 1;
    while (tok < tokens.size()) {
        size_t pos = tokens[tok].find('=');
        if (std::string::npos != pos) {
            std::string key = tokens[tok].substr(0, pos);
            std::string val = tokens[tok].substr(pos+1);
            setenv(key.c_str(), val.c_str(), 1);
            ++tok;
            LDEBUG1 << key << "=" << val;
        }
        else {
            break; /* no key=value found, break out */
        }
    }

    const char *program = tokens[tok].c_str();
    /* check that we can execute the program */
    if (!check_program(program)) {
        ERR << program << ": No such file or directory";
        return false;
    }

    return true;
}


pid_t unzip_command(std::string command)
{
    /* parse the command so we can call fork exec on the unzip */
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
        return -1;
    }
    /* attempt to change path now and report any error */
    std::string working_directory = tokens[0];
    if (working_directory == "pwd") {
        char buf[PATH_MAX];
        if (NULL == getcwd(buf, PATH_MAX)) {
            ERRNO << "getcwd";
            return -1;
        }
        working_directory = buf;
    }
    else {
        int retval = chdir(working_directory.c_str());
        if (0 != retval) {
            ERRNO << "chdir(" << working_directory << ")";
            return -1;
        }
    }

    /* find position of the command */
    size_t tok = 1;
    while (tok < tokens.size()) {
        size_t pos = tokens[tok].find('=');
        if (std::string::npos != pos) {
            ++tok;
        }
        else {
            break; /* no key=value found, break out */
        }
    }

    std::vector<char*> new_argv;
    for (; tok<tokens.size(); ++tok) {
        new_argv.push_back(strdup(tokens[tok].c_str()));
    }
    new_argv.push_back(NULL);

    pid_t cpid = fork();
    if (-1 == cpid) {
        /* fork error */
        ERRNO << "fork";
        return -1;
    } else if (0 == cpid) {
        /* this is the child */
        int retval;
        if (strcmp(new_argv[2],"gridlabd") == 0) { // if this is a GridLAB-D federate we need to extract the model file
            char const * const uzipcommand[] = { "unzip", "-qq", "*.zip", NULL };
            //retval = execvp(uzipcommand[0], uzipcommand);
            retval = execvp("unzip", (char**)uzipcommand);
            if (-1 == retval) {
                ERRNO << "execvp(unzip *.zip)";
                _exit(EXIT_FAILURE);
            }    
        } else { // my not so elegant way to ensure that we do not fork without executing a command...
            //const char* uzipcommand[] = { "true", "", NULL };
            char const * const uzipcommand[] = { "true", "", NULL };
            retval = execvp("true", (char**)uzipcommand);
            if (-1 == retval) {
                ERRNO << "execvp(true)";
                _exit(EXIT_FAILURE);
            }       	
        }
    }

    return cpid;
}



pid_t run_command(std::string command)
{
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
        return -1;
    }
    /* attempt to change path now and report any error */
	std::string working_directory = tokens[0];
    if (working_directory == "pwd") {
        char buf[PATH_MAX];
        if (NULL == getcwd(buf, PATH_MAX)) {
            ERRNO << "getcwd";
            return -1;
        }
        working_directory = buf;
    }
    else {
        int retval = chdir(working_directory.c_str());
        if (0 != retval) {
            ERRNO << "chdir(" << working_directory << ")";
            return -1;
        }
    }
    /* figure out the zipdir early, in case of error */
    std::string zipdir;
    if (0 != world_rank) {
        std::string::size_type found = working_directory.rfind('/');
        if (found == std::string::npos) {
            ERR << working_directory << ": could not determine directory to zip";
            return -1;
        }
        zipdir = working_directory.substr(found+1);
    }
    /* set any env vars from the input file */
    size_t tok = 1;
    while (tok < tokens.size()) {
        size_t pos = tokens[tok].find('=');
        if (std::string::npos != pos) {
            std::string key = tokens[tok].substr(0, pos);
            std::string val = tokens[tok].substr(pos+1);
            setenv(key.c_str(), val.c_str(), 1);
            ++tok;
            LDEBUG1 << key << "=" << val;
        }
        else {
            break; /* no key=value found, break out */
        }
    }

    const char *program = tokens[tok].c_str();
    /* check that we can execute the program */
    if (!check_program(program)) {
        ERR << program << ": No such file or directory";
        return -1;
    }

    std::vector<char*> new_argv;
    for (; tok<tokens.size(); ++tok) {
        new_argv.push_back(strdup(tokens[tok].c_str()));
    }
    new_argv.push_back(NULL);

    pid_t cpid = fork();
    if (-1 == cpid) {
        /* fork error */
        ERRNO << "fork";
        return -1;
    } else if (0 == cpid) {
        /* this is the child */
        int retval;
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
    }

    return cpid;
}


bool zip_command(std::string command, std::string archive_dir)
{
    /* parse the command so we can get working directory */
    /* command is parsed based on whitespace */
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

    /* attempt to change path now and report any error */
	std::string working_directory = tokens[0];
    if (working_directory == "pwd") {
        char buf[PATH_MAX];
        if (NULL == getcwd(buf, PATH_MAX)) {
            ERRNO << "getcwd";
            return true;
        }
        working_directory = buf;
    }
    else {
        int retval = chdir(working_directory.c_str());
        if (0 != retval) {
            ERRNO << "chdir(" << working_directory << ")";
            return true;
        }
    }
    /* figure out the zipdir early, in case of error */
    std::string zipdir;
    {
        std::string::size_type found = working_directory.rfind('/');
        if (found == std::string::npos) {
            ERR << working_directory << ": could not determine directory to zip";
            return true;
        }
        zipdir = working_directory.substr(found+1);
    }

    /* If we got this far, we can zip up our working directory
     * and store it at the given location. We rely on an external
     * zip or zip-like tool. */
    {
        std::string zipfile = zipdir + "." + ZIP_EXT;
        const char *program = ZIP;
        std::vector<char*> new_argv;
        tokens.clear();
        tokens.push_back(ZIP);
        tokens.push_back(ZIP_ARGS);
        tokens.push_back(zipfile);
        tokens.push_back(zipdir);

        /* debugging print of all assigned commands */
        LDEBUG1 << tokens[0]
            << " " << tokens[1]
            << " " << tokens[2]
            << " " << tokens[3];

        size_t tok = 0;
        for (; tok<tokens.size(); ++tok) {
            new_argv.push_back(strdup(tokens[tok].c_str()));
        }
        new_argv.push_back(NULL);

        /* working directory for archiving is up one diretory */
        int retval = chdir("..");
        if (0 != retval) {
            ERRNO << "chdir(..) for archive";
            return true;
        }

        pid_t cpid = fork();
        if (-1 == cpid) {
            /* fork error */
            ERRNO << "fork archive";
            return true;
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
                return true;
            }
            if (WIFEXITED(status)) {
                LDEBUG << "archive exited, status=" << WEXITSTATUS(status);
                if (0 != WEXITSTATUS(status)) {
                    return true;
                }
            } else if (WIFSIGNALED(status)) {               
                ERR << "archive killed by signal " << WTERMSIG(status) << " (" << command << ")";
                return true;
            } else if (WIFSTOPPED(status)) {             
                ERR << "archive stopped by signal " << WSTOPSIG(status) << " (" << command << ")";
                return true;
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

    return false;
}


/* Parses the given file containing our command list.
 *
 * The file has the format, one per line
 * [KEY1=VALUE1] ... [KEYN=VALUEN] program_name [arg1] ... [argn]
 */
std::vector<std::string> parse_input_file(const char *filename)
{
    std::vector<std::string> commands;
    int world_rank;
    int world_size;
    int total = 0;
    int count = 0;
    int remain = 0;
    int start = 0;
    int stop = 0;
    std::string myline;

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    /* All ranks open the file for parsing.
     * They pick out only their own lines. */
    {
        std::ifstream fin(filename);
        std::string line;
        /* input file might start with comment(s), skip them all */
        do {
            std::getline(fin, line);
        } while (fin.good() && line[0] == '#');
        if (line.empty() || line[0] == '#') {
            /* file was only comments */
            ERR << "Launcher input file missing actual data.";
            commands.clear();
            return commands;
        }
        /* all ranks read the whole file to count the lines */
        while (fin.good()) {
            ++total;
            getline(fin, line);
        }
    }
    /* only report bad files from rank 0 */
    if (0 == world_rank) {
        if (total < world_size-1) {
            ERR << "Launcher input file contained "
                << total << " lines and you requested "
                << world_size << " MPI ranks."
                << std::endl
                << "You requested too many MPI ranks."
                << std::endl
                << "You need " << total << " plus 1 for the broker.";
        }
    }

    /* rank 0 has the broker command */
    if (0 == world_rank) {
        std::ostringstream os;
        //os << "pwd fncs_broker " << total;
        os << "pwd fncs_broker " << total;
        commands.push_back(os.str());
    }

    /* divide lines evenly between ranks, spread remaineder out */
    count = total / world_size;
    remain = total % world_size;
    start = world_rank * count;
    stop = world_rank * count + count;
    if (world_rank < remain) {
        start += world_rank;
        stop += world_rank + 1;
    }
    else {
        start += remain;
        stop += remain;
    }
    LDEBUG1 << "start=" << start << " stop=" << stop;

    /* now reread the file and pull out the commands */
    {
        std::ifstream fin(filename);
        std::string line;
        /* input file might start with comment(s), skip them all */
        do {
            std::getline(fin, line);
        } while (fin.good() && line[0] == '#');
        if (line.empty() || line[0] == '#') {
            /* file was only comments */
            ERR << "Launcher input file missing actual data.";
            commands.clear();
            return commands;
        }
        /* all ranks read the whole file to count the lines */
        total = 0;
        while (fin.good()) {
            if (start <= total && total < stop) {
                commands.push_back(line);
            }
            ++total;
            getline(fin, line);
        }
    }


    return commands;
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


