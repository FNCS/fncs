#include "config.h"

#include "easylogging++.h"
_INITIALIZE_EASYLOGGINGPP
using namespace easyloggingpp;

#include "fncs.hpp"

int main(int argc, char **argv)
{
    fncs::time granted = 0;

    fncs::initialize();

    fncs::publish("key", "value");

    granted = fncs::time_request(10);

    fncs::finalize();

    return 0;
}

