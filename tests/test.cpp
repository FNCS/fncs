#include "config.h"

#include "easylogging++.h"
_INITIALIZE_EASYLOGGINGPP
using namespace easyloggingpp;

#include <cassert>

#include "fncs.hpp"

int main(int argc, char **argv)
{
    string value;
    vector<string> values;
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

#if 0
    /* should be updated cache after request */
    value = fncs::get_value("baz");
    values = fncs::get_values("bazl");
#endif

    fncs::publish("key", "value2");

    /* time request */
    granted = fncs::time_request(15);

#if 0
    /* cache lists empty now */
    values = fncs::get_values("bazl");
    assert(0 == values.size());
#endif

    fncs::publish("key2", "value");

    fncs::finalize();

    return 0;
}

