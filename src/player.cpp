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

    /* test default values work */
    value = fncs::get_value("baz");
    values = fncs::get_values("bazl");

    /* test sending of messages */
    fncs::publish("key", "value");
    fncs::route("objA", "objB", "key", "value");
    fncs::publish("yet", "again");

    /* time request */
    granted = fncs::time_request(10);

    /* should be updated cache after request */
    value = fncs::get_value("baz");
    values = fncs::get_values("bazl");

    /* time request */
    granted = fncs::time_request(15);

    /* cache lists empty now */
    values = fncs::get_values("bazl");
    assert(0 == values.size());

    fncs::finalize();

    return 0;
}

