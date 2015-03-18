#include "config.h"

#include "easylogging++.h"
_INITIALIZE_EASYLOGGINGPP
using namespace easyloggingpp;

#include <cstdlib>
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
using std::pair;
using std::string;
using std::vector;

typedef pair<string,string> kv_t;
typedef vector<kv_t> matches_t;

int main(int argc, char **argv)
{
    fncs::time time_granted = 0;
    fncs::time time_stop = 0;
    vector<pair<string,string> > matches;

    if (argc != 2) {
        cerr << "Missing stop time parameter." << endl;
        exit(EXIT_FAILURE);
    }

    fncs::initialize();

    time_stop = fncs::parse_time(argv[1]);
    cout << "stops at " << time_stop << " nanoseconds" << endl;
    time_stop = fncs::convert_broker_to_sim_time(time_stop);
    cout << "stops at " << time_stop << " in sim time" << endl;

    do {
        time_granted = fncs::time_request(time_stop);
        cout << "time_granted is " << time_granted << endl;
        matches = fncs::get_matches("all");
        for (matches_t::iterator it=matches.begin(); it!=matches.end(); ++it) {
            cout << time_granted
                << "\t" << it->first
                << "\t" << it->second
                << endl;;
        }
    } while (time_granted < time_stop);
    cout << "time_granted was " << time_granted << endl;
    cout << "time_stop was " << time_stop << endl;

    cout << "done" << endl;

    fncs::finalize();

    return 0;
}

