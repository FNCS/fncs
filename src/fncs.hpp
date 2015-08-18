/**
 * TODO.
 */
#ifndef _FNCS_HPP_
#define _FNCS_HPP_
 
#include <string>
#include <utility>
#include <vector>

using ::std::pair;
using ::std::string;
using ::std::vector;

#if defined _WIN32 && !defined __GNUC__
# ifdef LIBFNCS_BUILD
#  ifdef DLL_EXPORT
#   define LIBFNCS_SCOPE            __declspec (dllexport)
#   define LIBFNCS_SCOPE_VAR extern __declspec (dllexport)
#  endif
# elif defined _MSC_VER
#  define LIBFNCS_SCOPE
#  define LIBFNCS_SCOPE_VAR  extern __declspec (dllimport)
# elif defined DLL_EXPORT
#  define LIBFNCS_SCOPE             __declspec (dllimport)
#  define LIBFNCS_SCOPE_VAR  extern __declspec (dllimport)
# endif
#endif
#ifndef LIBFNCS_SCOPE
# define LIBFNCS_SCOPE
# define LIBFNCS_SCOPE_VAR extern
#endif

namespace fncs {

    typedef unsigned long long time;

    /** Connect to broker and parse config file. */
    LIBFNCS_SCOPE void initialize();

    /** Connect to broker and parse inline configuration. */
    LIBFNCS_SCOPE void initialize(const string &configuration);

    /** Request the next time step to process. */
    LIBFNCS_SCOPE time time_request(time next);

    /** Publish value using the given key. */
    LIBFNCS_SCOPE void publish(const string &key, const string &value);

    /** Publish value using the given key, adding from:to into the key. */
    LIBFNCS_SCOPE void route(const string &from, const string &to, const string &key, const string &value);

    /** Tell broker of a fatal client error. */
    LIBFNCS_SCOPE void die();

    /** Close the connection to the broker. */
    LIBFNCS_SCOPE void finalize();

    /** Update minimum time delta after connection to broker is made.
     * Assumes time unit is not changing. */
    LIBFNCS_SCOPE void update_time_delta(time delta);

    /** Get the keys for all values that were updated during the last
     * time_request. */
    LIBFNCS_SCOPE vector<string> get_events();

    /** Get a value from the cache with the given key.
     * Will hard fault if key is not found. */
    LIBFNCS_SCOPE string get_value(const string &key);

    /** Get a vector of values from the cache with the given key.
     * Will return a vector of size 1 if only a single value exists. */
    LIBFNCS_SCOPE vector<string> get_values(const string &key);

    /** Get a vector of topic-value pairs from the cache with the given key.
     * Will return a vector of size 1 if only a single value exists. */
    LIBFNCS_SCOPE vector<pair<string,string> > get_matches(const string &key);

    /** Return the name of the simulator. */
    LIBFNCS_SCOPE string get_name();

    /** Return a unique numeric ID for the simulator. */
    LIBFNCS_SCOPE int get_id();

    /** Return the number of simulators connected to the broker. */
    LIBFNCS_SCOPE int get_simulator_count();

}

#endif /* _FNCS_HPP_ */

