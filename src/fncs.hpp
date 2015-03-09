/**
 * TODO.
 */
#ifndef _FNCS_H_
#define _FNCS_H_
 
#include <string>
#include <vector>

using ::std::string;
using ::std::vector;

namespace fncs {

    typedef unsigned long long time;

    /** Connect to broker and parse config file. */
    void initialize();

    /** Connect to broker and parse inline configuration. */
    void initialize(const string &configuration);

    /** Request the next time step to process. */
    time time_request(time next);

    /** Publish value using the given key. */
    void publish(const string &key, const string &value);

    /** Publish value using the given key, adding from:to into the key. */
    void route(const string &from, const string &to, const string &key, const string &value);

    /** Tell broker of a fatal client error. */
    void die();

    /** Close the connection to the broker. */
    void finalize();

    /** Get a value from the cache with the given key.
     * Will hard fault if key is not found. */
    string get_value(const string &key);

    /** Get a vector of values from the cache with the given key.
     * Will return a vector of size 1 if only a single value exists. */
    vector<string> get_values(const string &key);

    /** Return the name of the simulator. */
    string get_name();

    /** Return a unique numeric ID for the simulator. */
    int get_id();

    /** Return the number of simulators connected to the broker. */
    int get_simulator_count();

}

#endif /* _FNCS_H_ */

