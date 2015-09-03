/* autoconf header */
#include "config.h"

/* C++ standard headers */
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <list>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

/* fncs headers */
#include "fncs.hpp"
#include "fncs_internal.hpp"

typedef pair<string,string> kv_t;
typedef pair<fncs::time,kv_t> event_t;

using namespace ::std;

vector<string> split(string str, char delimiter) {
    vector<string> retval;
    stringstream ss(str);
    string tok;
    while(getline(ss, tok, delimiter)) {
        retval.push_back(tok);
    }
    return retval;
}


int main(int argc, char **argv)
{
    string param_time_stop = "";
    string param_time_delay_min = "";
    string param_time_delay_max = "";
    fncs::time time_granted = 0;
    fncs::time time_stop = 0;
    fncs::time time_delay_min = 0;
    fncs::time time_delay_max = 0;
    fncs::time time_delay_range = 0;
    list<event_t> events;

    const char * usage = "Usage: fncs_netdelay <stop time> <delaymin> <delaymax>";

    if (argc < 2) {
        cerr << "Missing stop time." << endl;
        cerr << usage << endl;
        exit(EXIT_FAILURE);
    }
    
    if (argc < 3) {
        cerr << "Missing delay min time." << endl;
        cerr << usage << endl;
        exit(EXIT_FAILURE);
    }
    
    if (argc < 4) {
        cerr << "Missing delay max time." << endl;
        cerr << usage << endl;
        exit(EXIT_FAILURE);
    }
    
    if (argc > 5) {
        cerr << "Too many parameters." << endl;
        cerr << usage << endl;
        exit(EXIT_FAILURE);
    }

    param_time_stop = argv[1];
    param_time_delay_min = argv[2];
    param_time_delay_max = argv[3];

    {
        int value = -1;
        istringstream iss(param_time_delay_min);
        iss >> value;
        if (value < 0) {
            cerr << "delay min must be >= 0" << endl;
            exit(EXIT_FAILURE);
        }
        time_delay_min = static_cast<fncs::time>(value);
    }

    {
        int value = -1;
        istringstream iss(param_time_delay_max);
        iss >> value;
        if (value < 0) {
            cerr << "delay max must be >= 0" << endl;
            exit(EXIT_FAILURE);
        }
        time_delay_max = static_cast<fncs::time>(value);
    }

    if (time_delay_max < time_delay_min) {
        cerr << "delay min must be <= delay max" << endl;
        exit(EXIT_FAILURE);
    }

    time_delay_range = time_delay_max - time_delay_min + 1;

    fncs::initialize();

    time_stop = fncs::parse_time(param_time_stop);
    cout << "stops at " << time_stop << " nanoseconds" << endl;
    time_stop = fncs::convert_broker_to_sim_time(time_stop);
    cout << "stops at " << time_stop << " in sim time" << endl;

    time_granted = fncs::time_request(time_stop);

    while (time_granted < time_stop) {
        vector<string> event_labels = fncs::get_events();

        cout << "events (" << event_labels.size() << ") detected at time " << time_granted << endl;

        /* tokenize and delay events */
        if (!event_labels.empty()) {
            for (size_t i=0; i<event_labels.size(); ++i) {
                vector<string> parts = split(event_labels[i], '/');
                if (parts.size() != 3) {
                    cout << "skipping event " << event_labels[i] << endl;
                }
                else {
                    string value = fncs::get_value(event_labels[i]);
                    fncs::time time_add = rand()%time_delay_range + time_delay_min;
                    fncs::time new_time = time_granted + time_add;
                    cout << "delaying event " << event_labels[i] << " to " << new_time << endl;
                    events.push_back(make_pair(new_time,
                                make_pair(event_labels[i], value)));
                }
            }
            events.sort();
        }

        /* publish any events we might have */
        while (!events.empty() && events.front().first <= time_granted) {
            event_t event;
            event = events.front();
            events.pop_front();
            fncs::publish(event.second.first, event.second.second);
        }

        /* get next event time */
        if (events.empty()) {
            time_granted = fncs::time_request(time_stop);
        }
        else {
            time_granted = fncs::time_request(events.front().first);
        }
    }

    cout << "done" << endl;

    fncs::finalize();

    return 0;
}

