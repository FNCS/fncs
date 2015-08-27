/**
 * TODO.
 */
#ifndef _FNCS_H_
#define _FNCS_H_
 
#if defined (__WINDOWS__)
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

#ifdef __cplusplus
extern "C" {
#endif

    typedef unsigned long long fncs_time;

    /** Connect to broker and parse config file. */
    FNCS_EXPORT void fncs_initialize();

    /** Connect to broker and parse inline configuration. */
    FNCS_EXPORT void fncs_initialize_config(const char *configuration);

    /** Request the next time step to process. */
    FNCS_EXPORT fncs_time fncs_time_request(fncs_time next);

    /** Publish value using the given key. */
    FNCS_EXPORT void fncs_publish(const char *key, const char *value);

    /** Publish value using the given key, adding from:to into the key. */
    FNCS_EXPORT void fncs_route(
            const char *from,
            const char *to,
            const char *key,
            const char *value);

    /** Tell broker of a fatal client error. */
    FNCS_EXPORT void fncs_die();

    /** Close the connection to the broker. */
    FNCS_EXPORT void fncs_finalize();

    /** Update minimum time delta after connection to broker is made.
     * Assumes time unit is not changing. */
    FNCS_EXPORT void fncs_update_time_delta(fncs_time delta);

    /** Get the keys for all values that were updated during the last
     * time_request. */
    FNCS_EXPORT void fncs_get_events(char*** events, size_t *size);

    /** Get a value from the cache with the given key.
     * Will hard fault if key is not found. */
    FNCS_EXPORT char * fncs_get_value(const char *key);

    /** Get an array of values from the cache with the given key.
     * Will return an array of size 1 if only a single value exists. */
    FNCS_EXPORT void fncs_get_values(
            const char *key,
            char *** values,
            size_t *size);

    /** Get an array of topic-value pairs from the cache with the given key.
     * Will return an array of size 1 if only a single value exists. */
    FNCS_EXPORT void fncs_get_matches(
            const char *key,
            char *** topics,
            char *** values,
            size_t *size);

    /** Return the name of the simulator. */
    FNCS_EXPORT const char * fncs_get_name();

    /** Return a unique numeric ID for the simulator. */
    FNCS_EXPORT int fncs_get_id();

    /** Return the number of simulators connected to the broker. */
    FNCS_EXPORT int fncs_get_simulator_count();

#ifdef __cplusplus
}
#endif

#endif /* _FNCS_H_ */

