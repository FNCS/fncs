#include "config.h"

#include "easylogging++.h"
_INITIALIZE_EASYLOGGINGPP
using namespace easyloggingpp;

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "fncs.hpp"
#include "fncs_internal.hpp"

using std::cerr;
using std::cout;
using std::exit;
using std::endl;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

typedef pair<string,string> kv_t;
typedef vector<kv_t> matches_t;

int main(int argc, char **argv)
{
    string param_time_stop = "";
    string param_file_name = "";
    fncs::time time_granted = 0;
    fncs::time time_stop = 0;
    vector<pair<string,string> > matches;
    ofstream fout;

    if (argc < 3) {
        cerr << "Missing stop time and/or output filename parameters." << endl;
        cerr << "Usage: tracer <stop time> <output file>" << endl;
        exit(EXIT_FAILURE);
    }
    
    if (argc > 3) {
        cerr << "Too many parameters." << endl;
        cerr << "Usage: tracer <stop time> <output file>" << endl;
        exit(EXIT_FAILURE);
    }


    param_time_stop = argv[1];
    param_file_name = argv[2];

    fout.open(param_file_name.c_str());
    if (!fout) {
        cerr << "Could not open output file '" << param_file_name << "'." << endl;
        exit(EXIT_FAILURE);
    }

    fout << "#nanoseconds\ttopic\tvalue" << endl;

#if 0
    fncs::initialize();
#else
    fncs::initialize(
            "name = tracer\n"
            "time_delta = 1ns\n"
            "broker = tcp://localhost:5570\n"
            "matches\n"
            "    all\n"
            "        topic = .*\n"
            );
#endif

    time_stop = fncs::parse_time(argv[1]);
    cout << "stops at " << time_stop << " nanoseconds" << endl;
    time_stop = fncs::convert_broker_to_sim_time(time_stop);
    cout << "stops at " << time_stop << " in sim time" << endl;

    do {
        time_granted = fncs::time_request(time_stop);
        cout << "time_granted is " << time_granted << endl;
        matches = fncs::get_matches("all");
        for (matches_t::iterator it=matches.begin(); it!=matches.end(); ++it) {
            fout << time_granted
                << "\t" << it->first
                << "\t" << it->second
                << endl;;
        }
    } while (time_granted < time_stop);
    cout << "time_granted was " << time_granted << endl;
    cout << "time_stop was " << time_stop << endl;

    cout << "done" << endl;

    fout.close();

    fncs::finalize();

    return 0;
}

