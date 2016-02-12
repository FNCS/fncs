#include <fstream>
#include <iostream>
#include <sstream>

#include "fncs.hpp"
#include "fncs_internal.hpp"

using namespace std;


int main(int argc, char **argv)
{
    if (argc != 2) {
        cerr << "usage: config filename.[zpl,yaml]" << endl;
        return 1;
    }

    std::ifstream t(argv[1]);
    std::stringstream buffer;
    buffer << t.rdbuf();

    fncs::Config config = fncs::parse_config(buffer.str());
    cout << config.to_string() << endl;

    return 0;
}

