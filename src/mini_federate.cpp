/*******************************************************************************
Mini (computation-free) federate

This "simulator" is desinged to be used in testing FNCS performance by acting 
as a computation-free federate; that is, a federate that only sends and recieves
FNCS messages and does not perform any meaningful computation on the data.

The assumed co-sim architecture this federate is designed to support is
many-to-one/one-to-many. As such the input arguments can be used to specify
whether a particular instance of this federate is asking as the root or one of
the leaves in that architecture.
    Root: Recieves FNCS messages and echoes them back
    Leaf: Generates FNCS messages on a pre-defined basis.

Trevor Hardy
*******************************************************************************/


/* autoconf header */
#include "config.h"

/* C++ standard headers */
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

/* 3rd party headers */
#include "czmq.h"

/* fncs headers */
#include "fncs.hpp"
#include "fncs_internal.hpp"

using namespace ::std;


int main(int argc, char **argv)
{
/*******************************************************************************
Input arguments
    (0: program name - always mini_federate)
    1: Federate type
        - 'root' - echoes back received FNCS messages
        - 'leaf' - generates FNCS messages on a pre-defined interval
    2: Simulation stop time (ns) - Length of time being "simulated" (duration of
            co-sim). This value should be uniform across all mini_federates in test.
    3: Update interval (ns) - Frequency at which time requests and data exchanges
            will be made.
    4: Number of messages per update (leaf federate only) - Number of FNCS
            messages generated each time the mini_federates update
    5: Size of messages (bytes, leaf federate only) - Size of each message
            sent each time the mini_federates update.
*******************************************************************************/
    
    bool isRoot = FALSE;     // Setting a Boolean to make this frequent comparison operation faster
    string param_federate_type = "";
    string param_stop_time = "";
    string param_update_interval = "";
    string param_num_messages = "";
    string param_message_size = "";
    string value = "";
    fncs::time time_granted = 0;
    fncs::time time_stop = 0;
    vector<string> events;
    ofstream fout;
    ostream out(cout.rdbuf()); /* share cout's stream buffer */

    // Input parameter error-checking
    // argc = number of input arguments + 1 (first parameter, argv[0], is mini_federate)
    param_federate_type = argv[1];
    param_stop_time = argv[2];
    if param_federate_type.compare('root'){
        isRoot = TRUE;
        if (argc < 3) {
            cerr << "Missing paramters" << endl;
            cerr << "Usage: mini_federate root [simulation stop time]" << endl;
            exit(EXIT_FAILURE);
            }
        else if (argc > 3) {
            cerr << "Too many parameters." << endl;
            cerr << "Usage: mini_federate root [simulation stop time]" << endl;
            exit(EXIT_FAILURE);
            }
        else { //correct number of parameters
            }
    } else if param_federate_type.compare('leaf'){
        isRoot = FALSE;
        if (argc < 6) {
            cerr << "Missing paramters" << endl;
            cerr << "Usage: mini_federate leaf [simulation stop time]" <<\
                    "[update interval] [messages/update] [message size]" << endl;
            exit(EXIT_FAILURE);
        }else if (argc > 6) {
            cerr << "Too many parameters." << endl;
            cerr << "Usage: mini_federate leaf [simulation stop time]" <<\
                    "[update interval] [messages/update] [message size]" << endl;
            exit(EXIT_FAILURE);
        } else { //correct number of parameters
            param_update_interval = atoul(argv[3].c_str());
            param_num_messages = atoi(argv[4].c_str());
            param_message_size = atoi(argv[5].c_str());
        }
    } else {
        cerr << "First parameter must be 'leaf' or 'root'." << endl;
        exit(EXIT_FAILURE);
    }
    
    // Creating message of appropriate size outside the main FNCS loop
    //  to save on computation during time-critial operation
    
    for (int idx = 0; idx < param_message_size; idx++){
        value = value + "0";
    }
    cout << "Standard message:  " << message << endl;

    // Connecting to FNCS broker
    fncs::initialize();
    if (!fncs::is_initialized()) {
        cout << "did not connect to broker, exiting" << endl;
        fout.close();
        return EXIT_FAILURE;
    }

    time_stop = fncs::parse_time(param_stop_time);
    cout << "stops at " << time_stop << " nanoseconds" << endl;
    time_stop = fncs::convert_broker_to_sim_time(time_stop);
    cout << "stops at " << time_stop << " in sim time" << endl;


    // Starting co-sim
    time_granted = 0; // at start of co-sim
    do {
        
        // Making time request
        fncs::time event;
        if isRoot{
            // Always requests stop time of simulation since root only echos back
            //  packets; no endogenous activity
            time_granted = fncs::time_request(time_stop);
            }
        else{
            // Request one update_interval from the last granted time.
            requesting_time = fncs::time(param_update_interval) + time_granted;
            time_granted = fncs::time_request(requesting_time);
            }
        cout << "time_granted is " << time_granted << endl;
        
        // "Processing" received events at granted time
        events = fncs::get_events();
        for (vector<string>::iterator it=events.begin(); it!=events.end(); ++it) {
            key = *it;
            value = fncs::get_value(*it);
            out << time_granted
                << "\t" << *it
                << "\t" << value
                << endl;
            if isRoot{
                // Echoing back received message
                fncs::publish(key, value);
            }
            else{
                // Leaf does nothing with received messages
            }
           
            // Generating new messages at granted time
            if isRoot{
                // Root node never generates messages on its own, only echos back
                //  what has been received.
            }
            else{
                for(int idx = 0; idx < param_num_messages; idx++ ){
                    string key = "key" + to_string(idx);
                    fncs::publish(key, value);
                }
            }
        }
    } while (time_granted < time_stop);
    cout << "time_granted was " << time_granted << endl;
    cout << "time_stop was " << time_stop << endl;

    cout << "done" << endl;

    fout.close();

    fncs::finalize();

    return EXIT_SUCCESS;
}

