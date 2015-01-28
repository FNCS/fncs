#include "config.h"

#include "easylogging++.h"
_INITIALIZE_EASYLOGGINGPP
using namespace easyloggingpp;

#include "fncs.hpp"

int main(int argc, char **argv)
{
    Loggers::setFilename("test.log");
    Loggers::reconfigureAllLoggers(ConfigurationType::ToStandardOutput, "false");
    LINFO << "This is my first info";
    LTRACE << "This is my first trace";

    return 0;
}

