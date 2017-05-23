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
#include <numeric>

/* 3rd party headers */
#include "czmq.h"

/* fncs headers */
#include "log.hpp"
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
            
    If leaf:
    3: Update interval (ns) - Frequency at which time requests and data exchanges
            will be made.
    4: Number of messages per update (leaf federate only) - Number of FNCS
            messages generated each time the mini_federates update
    5: Size of messages (bytes, leaf federate only) - Size of each message
            sent each time the mini_federates update.
 
*******************************************************************************/
    
    bool isRoot = false;     // Setting a Boolean to make this frequent comparison operation faster
    string param_federate_type = "";
    string param_stop_time = "";
    fncs::time param_update_interval = 0;
    unsigned long param_num_messages = 0;
    unsigned long param_message_size = 0;
    string value = "";
    string key = "";
    string real_key = "";
    fncs::time target_time = 0;
    fncs::time time_granted = 0;
    fncs::time time_stop = 0;
    vector<string> events;
    ofstream fout;
    ostream out(cout.rdbuf()); /* share cout's stream buffer */
    string zpl_line = "";
    string zpl_param = "";
    string zpl_equals = "";
    string zpl_value = "";
    string fed_name = "";
    bool done = false;
    
    // Initializing logging
    fncs::start_logging();
    fncs::replicate_logging(FNCSLog::ReportingLevel(),
            Output2Tee::Stream1(), Output2Tee::Stream2());
    
    // Message receipt tracking to aid in debugging
    vector<int> message_receipt;
    int message_ID = 0;
    int leaf_num = 0;
    int message_num = 0;

    // Input parameter error-checking
    if (argc == 1){
        cerr << "First parameter must be 'leaf' or 'root'." << endl;
        exit(EXIT_FAILURE);
    }else if (argc == 2){
        cerr << "First parameter must be 'leaf' or 'root'." << endl;
        cerr << "Second parameter must be simulation stop time." << endl;
        exit(EXIT_FAILURE);
    }else{
        param_federate_type = argv[1];
        param_stop_time = argv[2];
        if (param_federate_type.compare("root") == 0){
            isRoot = true;
            if (argc < 3) {
                cerr << "Missing paramters" << endl;
                cerr << "Usage: mini_federate root [simulation stop time]ns" << endl;
                exit(EXIT_FAILURE);
                }
            else if (argc > 3) {
                cerr << "Too many parameters." << endl;
            cerr << "Usage: mini_federate root [simulation stop time]ns" << endl;
                exit(EXIT_FAILURE);
                }
            else { //correct number of parameters
                }
        } else if (param_federate_type.compare("leaf") == 0){
            isRoot = false;
            if (argc < 6) {
                cerr << "Missing paramters" << endl;
                cerr << "Usage: mini_federate leaf [simulation stop time]ns" <<\
                        "[update interval] [messages/update] [message size]" << endl;
                exit(EXIT_FAILURE);
            }else if (argc > 6) {
                cerr << "Too many parameters." << endl;
                cerr << "Usage: mini_federate leaf [simulation stop time]ns" <<\
                        "[update interval] [messages/update] [message size]" << endl;
                exit(EXIT_FAILURE);
            } else { //correct number of parameters
                param_update_interval = atol(argv[3]);
                param_num_messages = atol(argv[4]);
                param_message_size = atol(argv[5]);
            }
        } else {
            cerr << "First parameter must be 'leaf' or 'root'." << endl;
            exit(EXIT_FAILURE);
        }
    }
    
    // Creating message of appropriate size outside the main FNCS loop
    //  to save on computation during time-critial operation
    
    for (int idx = 0; idx < param_message_size; idx++){
        value = value + "1";
    }
    LDEBUG4 << "Standard topic value:  " << value << endl;


    // Reading in fncs.zpl to figure out what my federate name is
    ifstream readFile("fncs.zpl");
    while(getline(readFile,zpl_line) && !done)   {
        stringstream iss(zpl_line);
        getline(iss, zpl_param, ' ');
        getline(iss, zpl_equals, ' ');
        getline(iss, zpl_value, ' ');
        if (zpl_param.compare("name") == 0){
            fed_name = zpl_value;
            done = true;
        }
    }
    readFile.close();
    LDEBUG4 << "Federate name as read from .zpl:  " << fed_name << endl;
    //

    // Connecting to FNCS broker
    fncs::initialize();
    if (!fncs::is_initialized()) {
        cout << "did not connect to broker, exiting" << endl;
        fout.close();
        return EXIT_FAILURE;
    }

    time_stop = fncs::parse_time(param_stop_time);
    //cout << "stops at " << time_stop << " nanoseconds" << endl;
    time_stop = fncs::convert_broker_to_sim_time(time_stop);
    //cout << "stops at " << time_stop << " in sim time" << endl;


    // Initialializing message receipt vector based on the number
    //  of subscriptions.
    int num_keys = fncs::get_keys().size();
    message_receipt.resize(num_keys + 1);
    LDEBUG4 << "message_receipt size: " << message_receipt.size();
    for(std::vector<int>::iterator it = message_receipt.begin(); it != message_receipt.end(); ++it){
        *it = 0;
    }

    // Starting co-sim
    LDEBUG2 << "Starting co-sim...";
    target_time = param_update_interval; // at start of co-sim
    do {
        
        // Making time request
        fncs::time event;
        if (isRoot) {
            // Always requests stop time of simulation since root only echos back
            //  packets; no endogenous activity
            time_granted = fncs::time_request(time_stop);
            }
        else{
            // Request one update_interval from the last granted time.
            time_granted = fncs::time_request(target_time);
            LDEBUG4 << "Requesting time: " << target_time;
            LDEBUG4 << "Granted time: " << time_granted;
            }
        
        // time_request returned a non-zero value which indicates a valid
        //  time has been granted and the message_receipt should be updated
        //  accordingly
        if (time_granted > 0) {
            LDEBUG4 << "Receipt of valid time grant logged.";
            message_receipt.at(0) = 1;
        }
        
        // time_granted == 0 when the zmq_poll times out.
        //  This condition is assumed to only happend when the co-sim has hung for
        //  unknown reasons. Because of this, the message_recipt vectors are used
        //  to indicate which messages have not been received when the time-out happens
        //  to aid in debugging.
        //
        // This debugging technique, though general, is most useful when used with the
        //  mini_federate becuase the message exchange is so regimented that comprehensive
        //  message accounting is reasonable.
        
        
        if (time_granted == 0){
            
            // Saving a bit of time by checking to see if any messages are missing
            //  before identifying which ones.
            int receipt_sum = std::accumulate(message_receipt.begin(), message_receipt.end(), 0);
            LDEBUG4 << "receipt_sum: " << receipt_sum << " of " << num_keys + 1 ;
            
            //num_keys doesn't track time_granted as a message
            if (receipt_sum != (num_keys + 1)){
                if (isRoot) {
                    // When root have to figure this out for myself since parameters were not
                    //  passed in.
                    int num_leafs = fncs::get_simulator_count() - 1;
                    //LDEBUG4 << "num_leafs: " << num_leafs;
                    int num_messages = num_keys/num_leafs;
                    //LDEBUG4 << "num_messages: " << num_messages;
                    for(std::vector<int>::iterator it = message_receipt.begin(); it != message_receipt.end(); ++it){
                        if (*it == 0) {
                            int idx = it - message_receipt.begin();
                            LDEBUG4 << "idx: " << idx;
                            if (idx == 0) {
                                LERROR << "Message missing: time grant";
                            }
                            else {
                                leaf_num = int (idx/num_messages);
                                LDEBUG4 << "leaf_num: " << leaf_num;
                                message_num = idx % num_messages;
                                LERROR << "Message missing: leaf" << leaf_num <<"_key" << message_num;
                            }
                        }
                    }
                }
                else {
                    for(std::vector<int>::iterator it = message_receipt.begin(); it != message_receipt.end(); ++it){
                        if (*it == 0) {
                            int idx = it - message_receipt.begin();
                            LDEBUG4 << "idx: " << idx;
                            if (idx == 0) {
                                LERROR << "Message missing: time grant";
                            }
                            else {
                                LERROR << "Message missing: root_key" << idx;
                        }
                        }
                    }
                }
                fncs::die();
            }
        }

        
        
        // "Processing" received events at granted time
        events = fncs::get_events();
        LDEBUG3 << "Getting events...";
        for (vector<string>::iterator it=events.begin(); it!=events.end(); ++it) {
            key = *it;
            value = fncs::get_value(*it);
            //cout << time_granted
            //    << "\t" << key
            //    << "\t" << value
            //    << endl;
            size_t pos = key.find('/');
            real_key = key.substr(pos+1,string::npos);
            LDEBUG4 << "key: " << key;
            LDEBUG4 << "real_key: " << real_key;
            if (isRoot) {
                // Recording receipt of message to support co-sim debugging if the
                //  co-sim later hangs. Gotta parse the key so I know which message
                //  to confirm receipt of.
                // real_key is formatted as: leaf<leaf num>_key<message num>
                size_t start_pos = real_key.find('f');
                size_t end_pos = real_key.find('_');
                size_t length = end_pos - start_pos - 1;
                string leaf_str = real_key.substr(start_pos+1, length);
                //LDEBUG4 << "leaf_str: " << leaf_str;
                leaf_num = atoi(leaf_str.c_str());
                LDEBUG4 << "leaf_num: " << leaf_num;
                start_pos = real_key.find('y');
                string message_str = real_key.substr(start_pos+1,string::npos);
                //LDEBUG4 << "message_str: " << message_str;
                message_num = atoi(message_str.c_str());
                LDEBUG4 << "message_num: " << message_num;
                int message_idx =(leaf_num - 1) * 10 + message_num;
                LDEBUG4 << "message_idx: " << message_idx;
                message_receipt.at(message_idx) = 1;
                
                // Echoing back received message
                fncs::publish(real_key, value);
                LDEBUG4 << "Publishing message with key, value: " << real_key << "," << value;
            }
            else{
                // Recording receipt of message to support co-sim debugging if the
                //  co-sim later hangs. Gotta parse the key so I know which message
                //  to confirm receipt of.
                // real_key is formatted as: leaf<leaf num>_key<message num>
                size_t start_pos = real_key.find('y');
                string message_str = real_key.substr(start_pos+1,string::npos);
                message_num = atoi(message_str.c_str());
                LDEBUG4 << "message_num: " << message_num;
                message_receipt.at(message_num) = 1;
                
                // Leaf does nothing with received messages
            }
        }
        if (time_granted == target_time){
            LDEBUG3 << "Publishing FNCS messages...";
            // Generating new messages at granted time
            if (isRoot) {
                // Root node never generates messages on its own, only echos back
                //  what has been received.
            }
            else{
                for(int idx = 1; idx <= param_num_messages; idx++ ){
                    ostringstream oss;
                    oss << idx;
                    string key = fed_name + "_key" + oss.str();
                    fncs::publish(key, value);
                }
            }
        // Resetting receipt vector since we worked our way all the way
        //  through the received message list. time_grant is always the
        //  last message in the queue.
        // Just copying-and-pasting code from earlier because I'm lazy.
        for(std::vector<int>::iterator it = message_receipt.begin(); it != message_receipt.end(); ++it){
            *it = 0;
        }
            
        target_time = target_time + param_update_interval;
        }
        
    } while (time_granted < time_stop);
    //cout << "time_granted was " << time_granted << endl;
    //cout << "time_stop was " << time_stop << endl;

    

    fout.close();

    fncs::finalize();

    cout << fed_name << " is done." << endl;
    return EXIT_SUCCESS;
}

