/* autoconf header */
#include "config.h"

/* C++ standard headers */
#include <map>
#include <string>

/* 3rd party headers */
#include "easylogging++.h"
_INITIALIZE_EASYLOGGINGPP
#include <zmq.h>

/* fncs headers */
#include "fncs.hpp"

using namespace ::easyloggingpp;
using namespace ::std;


class SimulatorState {
    public:
        SimulatorState()
            : time_requested(0)
            , time_smallest_delta(0)
            , time_last_processed(0)
            , processing(true)
            , messages_pending(false)
        {}

        fncs::time time_requested;
        fncs::time time_smallest_delta;
        fncs::time time_last_processed;
        bool processing;
        bool messages_pending;
};


int main(int argc, char **argv)
{
    /* declare all variables */
    map<string,SimulatorState> simulator_states;
    map<string,fncs::time> time_actionable;
    fncs::time time_granted = 0;

    /* start our logger */
    Loggers::setFilename("fncs.log");
    Loggers::reconfigureAllLoggers(ConfigurationType::ToStandardOutput, "false");

    return 0;
}

