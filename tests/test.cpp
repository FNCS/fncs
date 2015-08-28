#include "config.h"

#include <cassert>
#include <iostream>

#include "fncs.hpp"

using std::cout;
using std::endl;

int main(int argc, char **argv)
{
    string value;
    vector<string> values;
    vector<string> events;
    fncs::time granted = 0;

    fncs::initialize();

#if 0
    /* test default values work */
    value = fncs::get_value("baz");
    values = fncs::get_values("bazl");
#endif

    /* test sending of messages */
    fncs::publish("key", "value");
    fncs::route("objA", "objB", "key", "value");
    fncs::publish("yet", "again");

    /* time request */
    granted = fncs::time_request(10);

    /* what messages were received? */
    events = fncs::get_events();
    for (vector<string>::iterator it=events.begin(); it!=events.end(); ++it) {
        cout << *it << endl;
    }

#if 0
    /* should be updated cache after request */
    value = fncs::get_value("baz");
    values = fncs::get_values("bazl");
#endif

    fncs::publish("key", "value2");

    /* time request */
    granted = fncs::time_request(15);

    /* what messages were received? */
    events = fncs::get_events();
    for (vector<string>::iterator it=events.begin(); it!=events.end(); ++it) {
        cout << *it << endl;
    }

#if 0
    /* cache lists empty now */
    values = fncs::get_values("bazl");
    assert(0 == values.size());
#endif

    fncs::publish("key2", "value");

    fncs::finalize();

    return 0;
}

