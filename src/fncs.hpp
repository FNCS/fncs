#ifndef _FNCS_HPP_
#define _FNCS_HPP_
 
#include <string>
#include <utility>
#include <vector>

using ::std::pair;
using ::std::string;
using ::std::vector;

#if (defined WIN32 || defined _WIN32)
#   if defined LIBFNCS_STATIC
#       define FNCS_EXPORT
#   elif defined LIBFNCS_EXPORTS
#       define FNCS_EXPORT __declspec(dllexport)
#   else
#       define FNCS_EXPORT __declspec(dllimport)
#   endif
#else
#   define FNCS_EXPORT
#endif

namespace fncs {

    typedef unsigned long long time;

    /** Connect to broker and parse config file. */
    FNCS_EXPORT void initialize();

    /** Connect to broker and parse inline configuration. */
    FNCS_EXPORT void initialize(const string &configuration);

    /** Request the next time step to process. */
    FNCS_EXPORT time time_request(time next);

    /** Publish value using the given key. */
    FNCS_EXPORT void publish(const string &key, const string &value);

    /** Publish value using the given key, adding from:to into the key. */
    FNCS_EXPORT void route(const string &from, const string &to, const string &key, const string &value);

    /** Tell broker of a fatal client error. */
    FNCS_EXPORT void die();

    /** Close the connection to the broker. */
    FNCS_EXPORT void finalize();

    /** Update minimum time delta after connection to broker is made.
     * Assumes time unit is not changing. */
    FNCS_EXPORT void update_time_delta(time delta);

    /** Get the keys for all values that were updated during the last
     * time_request. */
    FNCS_EXPORT vector<string> get_events();

    /** Get a value from the cache with the given key.
     * Will hard fault if key is not found. */
    FNCS_EXPORT string get_value(const string &key);

    /** Get a vector of values from the cache with the given key.
     * Will return a vector of size 1 if only a single value exists. */
    FNCS_EXPORT vector<string> get_values(const string &key);

    /** Get a vector of topic-value pairs from the cache with the given key.
     * Will return a vector of size 1 if only a single value exists. */
    FNCS_EXPORT vector<pair<string,string> > get_matches(const string &key);

    /** Return the name of the simulator. */
    FNCS_EXPORT string get_name();

    /** Return a unique numeric ID for the simulator. */
    FNCS_EXPORT int get_id();

    /** Return the number of simulators connected to the broker. */
    FNCS_EXPORT int get_simulator_count();

}

#endif /* _FNCS_HPP_ */

