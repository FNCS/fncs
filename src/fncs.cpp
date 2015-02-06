/* autoconf header */
#include "config.h"

/* C++  standard headers */

/* 3rd party headers */
#include "czmq.h"
#include "easylogging++.h"
_INITIALIZE_EASYLOGGINGPP

/* fncs headers */
#include "fncs.hpp"

using namespace ::easyloggingpp;
using namespace ::std;


static zctx_t *ctx = NULL;


void fncs::initialize()
{
    LTRACE << "";

    /* open and parse fncs configuration */
    LTRACE << "open and parse fncs.zpl configuration";
    zconfig_t *zconfig = zconfig_load("fncs.zpl");

    /* start our logger */
    Loggers::setFilename("fncs.log");
    Loggers::reconfigureAllLoggers(ConfigurationType::ToStandardOutput, "false");
}

