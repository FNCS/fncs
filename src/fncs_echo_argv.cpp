/* this program prints argv as well as 
 * the environment to stdout */
#include "config.h"

#include <iostream>
#include <sstream>

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "fncs.hpp"

int main(int argc, char **argv)
{
    /* for debugging, track the special "die" argv */
    bool kill_me = false;
    bool abort_me = false;

    std::cout << "============ argv ============" << std::endl;
    for (int i=0; i<argc; ++i) {
        std::cout << argv[i] << std::endl;
        if (0 == strncmp(argv[i],"die",3)) kill_me |= true;
        if (0 == strncmp(argv[i],"abort",5)) abort_me |= true;
    }
    if (kill_me) std::cout << "I WILL EXIT WITH NON-ZERO CODE" << std::endl;
    if (abort_me) std::cout << "I WILL ABORT" << std::endl;

    std::cout << "============ env =============" << std::endl;
    for (int i=0; NULL != environ[i]; ++i) {
        std::cout << environ[i] << std::endl;
    }

    std::cerr << "============ err =============" << std::endl;
    std::cerr << "This message was written to stderr." << std::endl;

#ifdef FNCS_INIT
    char *rank_str = getenv("FNCS_LAUNCHER_RANK");
    if (NULL == rank_str) {
        std::cerr << "FNCS_LAUNCHER_RANK not set" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::ostringstream os;
    os << "name = test" << rank_str << std::endl;
    os << "time_delta = 1ns" << std::endl;
    std::cerr << "============ init ============" << std::endl;
    std::cerr << os.str() << std::endl;
    fncs::initialize(os.str());
    fncs::finalize();
#endif

    if (abort_me) abort();
    if (kill_me) return EXIT_FAILURE;
    return 0;
}

