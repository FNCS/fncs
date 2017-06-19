#ifndef _FNCS_FAPI_H_
#define _FNCS_FAPI_H_
 
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

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

    /* Connect to broker and parse config file. */
    FNCS_EXPORT void fncs_initialize_();
    FNCS_EXPORT void FNCS_INITIALIZE_();
    FNCS_EXPORT void fncs_initialize__();
    FNCS_EXPORT void FNCS_INITIALIZE__();

    /* Connect to broker and parse inline configuration. */
    FNCS_EXPORT void fncs_initialize_config_(char *config, int len);
    FNCS_EXPORT void FNCS_INITIALIZE_CONFIG_(char *config, int len);
    FNCS_EXPORT void fncs_initialize_config__(char *config, int len);
    FNCS_EXPORT void FNCS_INITIALIZE_CONFIG__(char *config, int len);

    /* Check whether simulator is configured and connected to broker. */
    FNCS_EXPORT int fncs_is_initialized_();
    FNCS_EXPORT int FNCS_IS_INITIALIZED_();
    FNCS_EXPORT int fncs_is_initialized__();
    FNCS_EXPORT int FNCS_IS_INITIALIZED__();

    /* Request the next time step to process. */
    FNCS_EXPORT int64_t fncs_time_request_(int64_t *next);
    FNCS_EXPORT int64_t FNCS_TIME_REQUEST_(int64_t *next);
    FNCS_EXPORT int64_t fncs_time_request__(int64_t *next);
    FNCS_EXPORT int64_t FNCS_TIME_REQUEST__(int64_t *next);

    /* Publish value using the given key. */
    FNCS_EXPORT void fncs_publish_(char *key, char *value, int key_len, int value_len);
    FNCS_EXPORT void FNCS_PUBLISH_(char *key, char *value, int key_len, int value_len);
    FNCS_EXPORT void fncs_publish__(char *key, char *value, int key_len, int value_len);
    FNCS_EXPORT void FNCS_PUBLISH__(char *key, char *value, int key_len, int value_len);

    /* Publish value anonymously using the given key. */
    FNCS_EXPORT void fncs_publish_anon_(char *key, char *value, int key_len, int value_len);
    FNCS_EXPORT void FNCS_PUBLISH_ANON_(char *key, char *value, int key_len, int value_len);
    FNCS_EXPORT void fncs_publish_anon__(char *key, char *value, int key_len, int value_len);
    FNCS_EXPORT void FNCS_PUBLISH_ANON__(char *key, char *value, int key_len, int value_len);

    /* Publish value using the given key, adding from:to into the key. */
    FNCS_EXPORT void fncs_route_(char *from, char *to, char *key, char *value, int from_len, int to_len, int key_len, int value_len);
    FNCS_EXPORT void FNCS_ROUTE_(char *from, char *to, char *key, char *value, int from_len, int to_len, int key_len, int value_len);
    FNCS_EXPORT void fncs_route__(char *from, char *to, char *key, char *value, int from_len, int to_len, int key_len, int value_len);
    FNCS_EXPORT void FNCS_ROUTE__(char *from, char *to, char *key, char *value, int from_len, int to_len, int key_len, int value_len);

    /* Tell broker of a fatal client error. */
    FNCS_EXPORT void fncs_die_();
    FNCS_EXPORT void FNCS_DIE_();
    FNCS_EXPORT void fncs_die__();
    FNCS_EXPORT void FNCS_DIE__();

    /* Close the connection to the broker. */
    FNCS_EXPORT void fncs_finalize_();
    FNCS_EXPORT void FNCS_FINALIZE_();
    FNCS_EXPORT void fncs_finalize__();
    FNCS_EXPORT void FNCS_FINALIZE__();

    /* Update minimum time delta after connection to broker is made.
     * Assumes time unit is not changing. */
    FNCS_EXPORT void fncs_update_time_delta_(int64_t *delta);
    FNCS_EXPORT void FNCS_UPDATE_TIME_DELTA_(int64_t *delta);
    FNCS_EXPORT void fncs_update_time_delta__(int64_t *delta);
    FNCS_EXPORT void FNCS_UPDATE_TIME_DELTA__(int64_t *delta);

    /* Get the number of keys for all values that were updated during
     * the last time_request. */
    FNCS_EXPORT int64_t fncs_get_events_size_();
    FNCS_EXPORT int64_t FNCS_GET_EVENTS_SIZE_();
    FNCS_EXPORT int64_t fncs_get_events_size__();
    FNCS_EXPORT int64_t FNCS_GET_EVENTS_SIZE__();

    /* Get one key for the given event index that as updated during the
     * last time_request. */
    FNCS_EXPORT void fncs_get_event_at_(int64_t *index, char *event, int event_len);
    FNCS_EXPORT void FNCS_GET_EVENT_AT_(int64_t *index, char *event, int event_len);
    FNCS_EXPORT void fncs_get_event_at__(int64_t *index, char *event, int event_len);
    FNCS_EXPORT void FNCS_GET_EVENT_AT__(int64_t *index, char *event, int event_len);

    /* Get a value from the cache with the given key.
     * Will hard fault if key is not found. */
    FNCS_EXPORT void fncs_get_value_(char *key, char *value, int key_len, int value_len);
    FNCS_EXPORT void FNCS_GET_VALUE_(char *key, char *value, int key_len, int value_len);
    FNCS_EXPORT void fncs_get_value__(char *key, char *value, int key_len, int value_len);
    FNCS_EXPORT void FNCS_GET_VALUE__(char *key, char *value, int key_len, int value_len);

    /* Get the number of values from the cache with the given key. */
    FNCS_EXPORT int64_t fncs_get_values_size_(char *key, int key_len);
    FNCS_EXPORT int64_t FNCS_GET_VALUES_SIZE_(char *key, int key_len);
    FNCS_EXPORT int64_t fncs_get_values_size__(char *key, int key_len);
    FNCS_EXPORT int64_t FNCS_GET_VALUES_SIZE__(char *key, int key_len);

    /* Get a single value from the array of values for the given key. */
    FNCS_EXPORT void fncs_get_value_at_(char *key, int64_t *index, char *value, int key_len, int value_len);
    FNCS_EXPORT void FNCS_GET_VALUE_AT_(char *key, int64_t *index, char *value, int key_len, int value_len);
    FNCS_EXPORT void fncs_get_value_at__(char *key, int64_t *index, char *value, int key_len, int value_len);
    FNCS_EXPORT void FNCS_GET_VALUE_AT__(char *key, int64_t *index, char *value, int key_len, int value_len);

    /* Get the number of subscribed keys. */
    FNCS_EXPORT int64_t fncs_get_keys_size_();
    FNCS_EXPORT int64_t FNCS_GET_KEYS_SIZE_();
    FNCS_EXPORT int64_t fncs_get_keys_size__();
    FNCS_EXPORT int64_t FNCS_GET_KEYS_SIZE__();

    /* Get the subscribed key at the given index.
     * Will return NULL if fncs_get_keys_size() returns 0. */
    FNCS_EXPORT void fncs_get_key_at_(int64_t *index, char *key, int key_len);
    FNCS_EXPORT void FNCS_GET_KEY_AT_(int64_t *index, char *key, int key_len);
    FNCS_EXPORT void fncs_get_key_at__(int64_t *index, char *key, int key_len);
    FNCS_EXPORT void FNCS_GET_KEY_AT__(int64_t *index, char *key, int key_len);

    /* Return the name of the simulator. */
    FNCS_EXPORT void fncs_get_name_(char *name, int name_len);
    FNCS_EXPORT void FNCS_GET_NAME_(char *name, int name_len);
    FNCS_EXPORT void fncs_get_name__(char *name, int name_len);
    FNCS_EXPORT void FNCS_GET_NAME__(char *name, int name_len);

    /* Return a unique numeric ID for the simulator. */
    FNCS_EXPORT int fncs_get_id_();
    FNCS_EXPORT int FNCS_GET_ID_();
    FNCS_EXPORT int fncs_get_id__();
    FNCS_EXPORT int FNCS_GET_ID__();

    /* Return the number of simulators connected to the broker. */
    FNCS_EXPORT int fncs_get_simulator_count_();
    FNCS_EXPORT int FNCS_GET_SIMULATOR_COUNT_();
    FNCS_EXPORT int fncs_get_simulator_count__();
    FNCS_EXPORT int FNCS_GET_SIMULATOR_COUNT__();

    /* Run-time API version detection. */
    FNCS_EXPORT void fncs_get_version_(int *major, int *minor, int *patch);
    FNCS_EXPORT void FNCS_GET_VERSION_(int *major, int *minor, int *patch);
    FNCS_EXPORT void fncs_get_version__(int *major, int *minor, int *patch);
    FNCS_EXPORT void FNCS_GET_VERSION__(int *major, int *minor, int *patch);

#ifdef __cplusplus
}
#endif

#endif /* _FNCS_FAPI_H_ */

