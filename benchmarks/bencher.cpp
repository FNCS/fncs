/*
 * bencher
 *
 * usage: bencher <ticks> <topics> <message size>
 *
 */
#include <cstdlib>
#include <iostream>
#include <sstream>

#include <fncs.hpp>

using namespace std;

const string LOWER("abcdefghijklmnopqrstuvwxyz");


int main(int argc, char **argv)
{
    const char *env_FNCS_LOG_LEVEL = getenv("FNCS_LOG_LEVEL");
    string FNCS_LOG_LEVEL = env_FNCS_LOG_LEVEL ? string(env_FNCS_LOG_LEVEL) : "";
    unsigned long long ticks = 0;
    unsigned long long topics = 0;
    unsigned long long size = 0;
    vector<string> KEY;

    if (4 != argc) {
        cout << "usage: bencher <ticks> <topics> <message size>" << endl;
        exit(EXIT_FAILURE);
    }

    {
        istringstream iticks(argv[1]);
        iticks >> ticks;
    }

    {
        istringstream itopics(argv[2]);
        itopics >> topics;
        if (topics > LOWER.size()*LOWER.size()) {
            cerr << "topics must be < " << LOWER.size()*LOWER.size() << endl;
            exit(EXIT_FAILURE);
        }
    }

    {
        istringstream isize(argv[3]);
        isize >> size;
    }

    {
        if (FNCS_LOG_LEVEL == "DEBUG4") {
            cout << "ticks = " << ticks << endl;
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

    fncs::time delta = fncs::get_time_delta();

    for (size_t tick = 0; tick < ticks; ++tick) {
        vector<string> events = fncs::get_events();
        /* get (all) events */
        for (size_t event = 0; event < events.size(); ++event) {
            (void)fncs::get_value(events[event]);
        }
        /* make events */
        for (size_t topic = 0; topic < topics; ++topic) {
            fncs::publish(KEY[topic], string(size, 'J'));
        }

        fncs::time requested = (tick+1)*delta;
        fncs::time granted = fncs::time_request(requested);
        if (FNCS_LOG_LEVEL == "DEBUG4") {
            cout << "time requested " << requested << " granted " << granted << endl;
        }
    }

    fncs::finalize();

    return 0;
}
