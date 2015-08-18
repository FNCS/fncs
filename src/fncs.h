/**
 * TODO.
 */
#ifndef _FNCS_H_
#define _FNCS_H_
 
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

#ifdef __cplusplus
extern "C" {
#endif

    typedef unsigned long long fncs_time;

    /** Connect to broker and parse config file. */
    LIBFNCS_SCOPE void fncs_initialize();

    /** Connect to broker and parse inline configuration. */
    LIBFNCS_SCOPE void fncs_initialize_config(const char *configuration);

    /** Request the next time step to process. */
    LIBFNCS_SCOPE fncs_time fncs_time_request(fncs_time next);

    /** Publish value using the given key. */
    LIBFNCS_SCOPE void fncs_publish(const char *key, const char *value);

    /** Publish value using the given key, adding from:to into the key. */
    LIBFNCS_SCOPE void fncs_route(
            const char *from,
            const char *to,
            const char *key,
            const char *value);

    /** Tell broker of a fatal client error. */
    LIBFNCS_SCOPE void fncs_die();

    /** Close the connection to the broker. */
    LIBFNCS_SCOPE void fncs_finalize();

    /** Update minimum time delta after connection to broker is made.
     * Assumes time unit is not changing. */
    LIBFNCS_SCOPE void fncs_update_time_delta(fncs_time delta);

    /** Get the keys for all values that were updated during the last
     * time_request. */
    LIBFNCS_SCOPE void fncs_get_events(char*** events, size_t *size);

    /** Get a value from the cache with the given key.
     * Will hard fault if key is not found. */
    LIBFNCS_SCOPE char * fncs_get_value(const char *key);

    /** Get an array of values from the cache with the given key.
     * Will return an array of size 1 if only a single value exists. */
    LIBFNCS_SCOPE void fncs_get_values(
            const char *key,
            char *** values,
            size_t *size);

    /** Get an array of topic-value pairs from the cache with the given key.
     * Will return an array of size 1 if only a single value exists. */
    LIBFNCS_SCOPE void fncs_get_matches(
            const char *key,
            char *** topics,
            char *** values,
            size_t *size);

    /** Return the name of the simulator. */
    LIBFNCS_SCOPE const char * fncs_get_name();

    /** Return a unique numeric ID for the simulator. */
    LIBFNCS_SCOPE int fncs_get_id();

    /** Return the number of simulators connected to the broker. */
    LIBFNCS_SCOPE int fncs_get_simulator_count();

#ifdef __cplusplus
}
#endif

#endif /* _FNCS_H_ */

