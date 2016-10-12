/*
 * bencher
 *
 * usage: bencher2 <stoptime> <maxticks> <topics> <message size>
 *
 * stoptime -- for example, 10s, 2h, 4days, 245nanoseconds
 * maxticks -- will request to move forward at most maxticks at a time, random
 *
 *
 */
#include <cstdlib>
#include <iostream>
#include <sstream>

#include <fncs.hpp>
#include <fncs_internal.hpp>

using namespace std;

const string LOWER("abcdefghijklmnopqrstuvwxyz");


int main(int argc, char **argv)
{
    const char *env_FNCS_LOG_LEVEL = getenv("FNCS_LOG_LEVEL");
    string FNCS_LOG_LEVEL = env_FNCS_LOG_LEVEL ? string(env_FNCS_LOG_LEVEL) : "";
    unsigned long long stoptime = 0;
    unsigned long long maxticks = 0;
    unsigned long long topics = 0;
    unsigned long long size = 0;
    vector<string> KEY;

    if (5 != argc) {
        cout << "usage: bencher2 <stoptime> <maxticks> <topics> <message size>" << endl;
        exit(EXIT_FAILURE);
    }

    {
        stoptime = fncs::parse_time(argv[1]);
    }

    {
        istringstream imaxticks(argv[2]);
        imaxticks >> maxticks;
    }

    {
        istringstream itopics(argv[3]);
        itopics >> topics;
        if (topics > LOWER.size()*LOWER.size()) {
            cerr << "topics must be < " << LOWER.size()*LOWER.size() << endl;
            exit(EXIT_FAILURE);
        }
    }

    {
        istringstream isize(argv[4]);
        isize >> size;
    }

    {
        if (FNCS_LOG_LEVEL == "DEBUG4") {
            cout << "stoptime = " << stoptime << " nanoseconds" << endl;
            cout << "maxticks = " << maxticks << endl;
            cout << "topics = " << topics << endl;
            cout << "size = " << size << endl;
        }
    }

    for (size_t i = 0; i < LOWER.size(); ++i) {
        for (size_t j = 0; j < LOWER.size(); ++j) {
            KEY.push_back(string()+LOWER[i]+LOWER[j]);
        }
    }

    fncs::initialize();

    fncs::time clock = 0;
    fncs::time delta = fncs::get_time_delta();
    stoptime = fncs::convert_broker_to_sim_time(stoptime);

    if (FNCS_LOG_LEVEL == "DEBUG4") {
        cout << "new stoptime = " << stoptime << " units" << endl;
    }

    while (clock < stoptime) {
        vector<string> events = fncs::get_events();
        /* get (all) events */
        for (size_t event = 0; event < events.size(); ++event) {
            (void)fncs::get_value(events[event]);
        }
        /* make events */
        for (size_t topic = 0; topic < topics; ++topic) {
            fncs::publish(KEY[topic], string(size, 'J'));
        }

        fncs::time increment = 1 + rand()%maxticks;
        fncs::time requested = clock + delta*increment;
        fncs::time granted = fncs::time_request(requested);
        if (FNCS_LOG_LEVEL == "DEBUG4") {
            cout << "time requested " << requested << " granted " << granted << endl;
        }
        clock = granted;
    }

    fncs::finalize();

    return 0;
}

