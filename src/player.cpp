/* autoconf header */
#include "config.h"

/* C++ standard headers */
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

/* 3rd party headers */
#include "czmq.h"
#include "easylogging++.h"
//_INITIALIZE_EASYLOGGINGPP

/* fncs headers */
#include "fncs.hpp"
#include "fncs_internal.hpp"

using namespace ::easyloggingpp;
using namespace ::std;

typedef pair<string,string> kv_t;
typedef vector<kv_t> matches_t;

int main(int argc, char **argv)
{
    string param_time_stop = "";
    string param_file_name = "";
    fncs::time time_granted = 0;
    fncs::time time_stop = 0;
    vector<pair<string,string> > matches;
    ifstream fin;
    string line = "";
    size_t counter = 0;

#ifdef FNCS_ANON
    const char * usage = "Usage: fncs_player_anon <stop time> <input file>";
#else
    const char * usage = "Usage: fncs_player <stop time> <input file>";
#endif

    if (argc < 3) {
        cerr << "Missing stop time and/or input filename parameters." << endl;
        cerr << usage << endl;
        exit(EXIT_FAILURE);
    }
    
    if (argc > 3) {
        cerr << "Too many parameters." << endl;
        cerr << usage << endl;
        exit(EXIT_FAILURE);
    }

    param_time_stop = argv[1];
    param_file_name = argv[2];

    fin.open(param_file_name.c_str());
    if (!fin) {
        cerr << "Could not open output file '" << param_file_name << "'." << endl;
        exit(EXIT_FAILURE);
    }

    fncs::initialize(
#ifdef FNCS_ANON
            "name = player_anon\n"
#else
            "name = player\n"
#endif
            "time_delta = 1ns\n"
            "broker = tcp://localhost:5570\n"
            );

    time_stop = fncs::parse_time(param_time_stop);
    cout << "stops at " << time_stop << " nanoseconds" << endl;
    time_stop = fncs::convert_broker_to_sim_time(time_stop);
    cout << "stops at " << time_stop << " in sim time" << endl;

    /* input file might start with comment(s), skip them all */
    do {
        getline(fin, line);
        ++counter;
    } while (fin.good() && line[0] == '#');

    if (line.empty() || line[0] == '#') {
        /* file was only comments */
        cerr << "Player input file missing actual data." << endl;
        fncs::die();
    }

    while (fin.good() && time_granted < time_stop) {
        fncs::time event;

        /* tokenize line */
        vector<string> tokens;
        istringstream iss(line);
        copy(istream_iterator<string>(iss),
                istream_iterator<string>(),
                back_inserter(tokens));
        if (tokens.size() != 3) {
            cerr << "Bad line: " << counter << ": '" << line << "'" << endl;
            fncs::die();
        }

        /* convert first token into fncs::time */
        {
            istringstream iss(tokens[0]);
            iss >> event;
            if (!iss) {
                cerr << "Bad time token in line: " << counter << ": '" << line << "'" << endl;
                fncs::die();
            }
        }

        /* sync */
        if (event < time_granted) {
            cerr << "Bad time token in line: " << counter << ": '" << line << "'" << endl;
            cerr << "Time value is smaller than previously granted time." << endl;
            fncs::die();
        }
        if (event > time_granted) {
            time_granted = fncs::time_request(event);
        }

        /* create event */
#ifdef FNCS_ANON
        fncs::publish_anon(tokens[1], tokens[2]);
#else
        fncs::publish(tokens[1], tokens[2]);
#endif

        /* read next line */
        getline(fin, line);
        ++counter;
    }

    cout << "done" << endl;

    fin.close();

    fncs::finalize();

    return 0;
}

