#include <cassert>
#include <string>

#include "fncs.hpp"
#include "fncs_fapi.h"
#include "log.hpp"

using std::string;


/* start at end of FORTRAN string and find first non-blank, copy to
 * std::string */
static inline string copy_f_to_cxx_string(char * s, int len)
{
    size_t i = len-1;
    while (s[i] == ' ' && i>0) { --i; }
    return string(s, i+1);
}

/* copy std::string into FORTRAN character buffer and fill with blanks */
static inline void copy_cxx_string_to_f(const string &str, char * s, int len)
{
    str.copy(s, str.size());
    for (int i=str.size(); i < len; i++) {
        s[i] = ' ';
    }
}

static inline void ffncs_initialize()
{
    fncs::initialize();
}
void fncs_initailize_() { ffncs_initialize(); }
void FNCS_INITAILIZE_() { ffncs_initialize(); }
void fncs_initailize__() { ffncs_initialize(); }
void FNCS_INITAILIZE__() { ffncs_initialize(); }

static inline void ffncs_initialize_config(char * config, int config_len)
{
    string str_config = copy_f_to_cxx_string(config, config_len);
    fncs::initialize(str_config);
}
void fncs_initialize_config_(char * config, int config_len) { ffncs_initialize_config(config, config_len); }
void FNCS_INITIALIZE_CONFIG_(char * config, int config_len) { ffncs_initialize_config(config, config_len); }
void fncs_initialize_config__(char * config, int config_len) { ffncs_initialize_config(config, config_len); }
void FNCS_INITIALIZE_CONFIG__(char * config, int config_len) { ffncs_initialize_config(config, config_len); }

static inline int ffncs_is_initialized()
{
    return fncs::is_initialized() ? 1 : 0;
}
int fncs_is_initialized_() { return ffncs_is_initialized(); }
int FNCS_IS_INITIALIZED_() { return ffncs_is_initialized(); }
int fncs_is_initialized__() { return ffncs_is_initialized(); }
int FNCS_IS_INITIALIZED__() { return ffncs_is_initialized(); }

static inline int64_t ffncs_time_request(int64_t * next)
{
    if (*next < 0) {
        LERROR << "FORTRAN: fncs_time_request next < 0";
        fncs::die();
        return 0;
    }
    return fncs::time_request(*next);
}
int64_t fncs_time_request_(int64_t * next) { return ffncs_time_request(next); }
int64_t FNCS_TIME_REQUEST_(int64_t * next) { return ffncs_time_request(next); }
int64_t fncs_time_request__(int64_t * next) { return ffncs_time_request(next); }
int64_t FNCS_TIME_REQUEST__(int64_t * next) { return ffncs_time_request(next); }

static inline void ffncs_publish(char * key, char * value, int key_len, int value_len)
{
    string str_key = copy_f_to_cxx_string(key, key_len);
    string str_value = copy_f_to_cxx_string(value, value_len);
    fncs::publish(str_key, str_value);
}
#if F2C_HIDDEN_STRING_LENGTH_AFTER_EACH
#define ARGS char * key, int key_len, char * value, int value_len
#else
#define ARGS char * key, char * value, int key_len, int value_len
#endif
void fncs_publish_(ARGS) { ffncs_publish(key, value, key_len, value_len); }
void FNCS_PUBLISH_(ARGS) { ffncs_publish(key, value, key_len, value_len); }
void fncs_publish__(ARGS) { ffncs_publish(key, value, key_len, value_len); }
void FNCS_PUBLISH__(ARGS) { ffncs_publish(key, value, key_len, value_len); }
#undef ARGS

static inline void ffncs_publish_anon(char * key, char * value, int key_len, int value_len)
{
    string str_key = copy_f_to_cxx_string(key, key_len);
    string str_value = copy_f_to_cxx_string(value, value_len);
    fncs::publish_anon(str_key, str_value);
}
#if F2C_HIDDEN_STRING_LENGTH_AFTER_EACH
#define ARGS char * key, int key_len, char * value, int value_len
#else
#define ARGS char * key, char * value, int key_len, int value_len
#endif
void fncs_publish_anon_(ARGS) { ffncs_publish_anon(key, value, key_len, value_len); }
void FNCS_PUBLISH_ANON_(ARGS) { ffncs_publish_anon(key, value, key_len, value_len); }
void fncs_publish_anon__(ARGS) { ffncs_publish_anon(key, value, key_len, value_len); }
void FNCS_PUBLISH_ANON__(ARGS) { ffncs_publish_anon(key, value, key_len, value_len); }
#undef ARGS

static inline void ffncs_route(char * from, char * to, char * key, char * value, int from_len, int to_len, int key_len, int value_len)
{
    string str_from = copy_f_to_cxx_string(from, from_len);
    string str_to = copy_f_to_cxx_string(to, to_len);
    string str_key = copy_f_to_cxx_string(key, key_len);
    string str_value = copy_f_to_cxx_string(value, value_len);
    fncs::route(str_from, str_to, str_key, str_value);
}
#if F2C_HIDDEN_STRING_LENGTH_AFTER_EACH
#define ARGS char * from, int from_len, char * to, int to_len, char * key, int key_len, char * value, int value_len
#else
#define ARGS char * from, char * to, char * key, char * value, int from_len, int to_len, int key_len, int value_len
#endif
void fncs_route_(ARGS) { ffncs_route(from, to, key, value, from_len, to_len, key_len, value_len); }
void FNCS_ROUTE_(ARGS) { ffncs_route(from, to, key, value, from_len, to_len, key_len, value_len); }
void fncs_route__(ARGS) { ffncs_route(from, to, key, value, from_len, to_len, key_len, value_len); }
void FNCS_ROUTE__(ARGS) { ffncs_route(from, to, key, value, from_len, to_len, key_len, value_len); }
#undef ARGS

static inline void ffncs_die()
{
    fncs::die();
}
void fncs_die_() { ffncs_die(); }
void FNCS_DIE_() { ffncs_die(); }
void fncs_die__() { ffncs_die(); }
void FNCS_DIE__() { ffncs_die(); }

static inline void ffncs_finalize()
{
    fncs::finalize();
}
void fncs_finalize_() { ffncs_finalize(); }
void FNCS_FINALIZE_() { ffncs_finalize(); }
void fncs_finalize__() { ffncs_finalize(); }
void FNCS_FINALIZE__() { ffncs_finalize(); }

static inline void ffncs_update_time_delta(int64_t * delta)
{
    if (*delta < 0) {
        LERROR << "FORTRAN: fncs_update_time_delta delta < 0";
        fncs::die();
    }
    fncs::update_time_delta(static_cast<fncs::time>(*delta));
}
void fncs_update_time_delta_(int64_t * delta) { ffncs_update_time_delta(delta); }
void FNCS_UPDATE_TIME_DELTA_(int64_t * delta) { ffncs_update_time_delta(delta); }
void fncs_update_time_delta__(int64_t * delta) { ffncs_update_time_delta(delta); }
void FNCS_UPDATE_TIME_DELTA__(int64_t * delta) { ffncs_update_time_delta(delta); }

static inline int64_t ffncs_get_events_size()
{
    return static_cast<int64_t>(fncs::get_events().size());
}
int64_t fncs_get_events_size_() { return ffncs_get_events_size(); }
int64_t FNCS_GET_EVENTS_SIZE_() { return ffncs_get_events_size(); }
int64_t fncs_get_events_size__() { return ffncs_get_events_size(); }
int64_t FNCS_GET_EVENTS_SIZE__() { return ffncs_get_events_size(); }

static inline void ffncs_get_event_at(int64_t * index, char * event, int event_len)
{
    int64_t cxx_index = *index - 1;
    if (cxx_index < 0) {
        LERROR << "FORTRAN: fncs_get_event_at index <= 0";
        fncs::die();
        return;
    }
    string cxx_event = fncs::get_events()[cxx_index];
    if (cxx_event.size() > event_len) {
        LERROR << "FORTRAN: fncs_get_event_at event(len=" << cxx_event.size()
            << ") does not fit into len=" << event_len << " character array";
        fncs::die();
        return;
    }
    copy_cxx_string_to_f(cxx_event, event, event_len);
}
void fncs_get_event_at_(int64_t * index, char * event, int event_len) { ffncs_get_event_at(index, event, event_len); }
void FNCS_GET_EVENT_AT_(int64_t * index, char * event, int event_len) { ffncs_get_event_at(index, event, event_len); }
void fncs_get_event_at__(int64_t * index, char * event, int event_len) { ffncs_get_event_at(index, event, event_len); }
void FNCS_GET_EVENT_AT__(int64_t * index, char * event, int event_len) { ffncs_get_event_at(index, event, event_len); }

static inline void ffncs_get_value(char * key, char * value, int key_len, int value_len)
{
    string str_key = copy_f_to_cxx_string(key, key_len);
    string cxx_value = fncs::get_value(str_key);
    if (cxx_value.size() > value_len) {
        LERROR << "FORTRAN: fncs_get_value value(len=" << cxx_value.size()
            << ") does not fit into len=" << value_len << " character array";
        fncs::die();
        return;
    }
    copy_cxx_string_to_f(cxx_value, value, value_len);
}
#if F2C_HIDDEN_STRING_LENGTH_AFTER_EACH
#define ARGS char * key, int key_len, char * value, int value_len
#else
#define ARGS char * key, char * value, int key_len, int value_len
#endif
void fncs_get_value_(ARGS) { ffncs_get_value(key, value, key_len, value_len); }
void FNCS_GET_VALUE_(ARGS) { ffncs_get_value(key, value, key_len, value_len); }
void fncs_get_value__(ARGS) { ffncs_get_value(key, value, key_len, value_len); }
void FNCS_GET_VALUE__(ARGS) { ffncs_get_value(key, value, key_len, value_len); }
#undef ARGS

static inline int64_t ffncs_get_values_size(char * key, int key_len)
{
    string str_key = copy_f_to_cxx_string(key, key_len);
    return fncs::get_values(str_key).size();
}
int64_t fncs_get_values_size_(char * key, int key_len) { return ffncs_get_values_size(key, key_len); }
int64_t FNCS_GET_VALUES_SIZE_(char * key, int key_len) { return ffncs_get_values_size(key, key_len); }
int64_t fncs_get_values_size__(char * key, int key_len) { return ffncs_get_values_size(key, key_len); }
int64_t FNCS_GET_VALUES_SIZE__(char * key, int key_len) { return ffncs_get_values_size(key, key_len); }

static inline void ffncs_get_value_at(char * key, int64_t * index, char * value, int key_len, int value_len)
{
    int64_t cxx_index = *index - 1;
    if (cxx_index < 0) {
        LERROR << "FORTRAN: fncs_get_value_at index <= 0";
        fncs::die();
        return;
    }
    string str_key = copy_f_to_cxx_string(key, key_len);
    if (cxx_index < fncs::get_values(str_key).size()) {
        LERROR << "FORTRAN: fncs_get_value_at index >= values size";
        fncs::die();
        return;
    }
    string cxx_value = fncs::get_values(str_key)[cxx_index];
    if (cxx_value.size() > value_len) {
        LERROR << "FORTRAN: fncs_get_value_at value(len=" << cxx_value.size()
            << ") does not fit into len=" << value_len << " character array";
        fncs::die();
        return;
    }
    copy_cxx_string_to_f(cxx_value, value, value_len);
}
#if F2C_HIDDEN_STRING_LENGTH_AFTER_EACH
#define ARGS char *key, int key_len, int64_t * index, char *value, int value_len
#else
#define ARGS char *key, int64_t * index, char *value, int key_len, int value_len
#endif
void fncs_get_value_at_(ARGS) { ffncs_get_value_at(key, index, value, key_len, value_len); }
void FNCS_GET_VALUE_AT_(ARGS) { ffncs_get_value_at(key, index, value, key_len, value_len); }
void fncs_get_value_at__(ARGS) { ffncs_get_value_at(key, index, value, key_len, value_len); }
void FNCS_GET_VALUE_AT__(ARGS) { ffncs_get_value_at(key, index, value, key_len, value_len); }
#undef ARGS

static inline int64_t ffncs_get_keys_size()
{
    return fncs::get_keys().size();
}
int64_t fncs_get_keys_size_() { return ffncs_get_keys_size(); }
int64_t FNCS_GET_KEYS_SIZE_() { return ffncs_get_keys_size(); }
int64_t fncs_get_keys_size__() { return ffncs_get_keys_size(); }
int64_t FNCS_GET_KEYS_SIZE__() { return ffncs_get_keys_size(); }

static inline void ffncs_get_key_at(int64_t * index, char * key, int key_len)
{
    int64_t cxx_index = *index - 1;
    if (cxx_index < 0) {
        LERROR << "FORTRAN: fncs_get_key_at index <= 0";
        fncs::die();
        return;
    }
    if (cxx_index < fncs::get_keys().size()) {
        LERROR << "FORTRAN: fncs_get_key_at index >= keys size";
        fncs::die();
        return;
    }
    string cxx_key = fncs::get_keys()[cxx_index];
    if (cxx_key.size() > key_len) {
        LERROR << "FORTRAN: fncs_get_value_at value(len=" << cxx_key.size()
            << ") does not fit into len=" << key_len << " character array";
        fncs::die();
        return;
    }
    copy_cxx_string_to_f(cxx_key, key, key_len);
}
void fncs_get_key_at_(int64_t * index, char * key, int key_len) { ffncs_get_key_at(index, key, key_len); }
void FNCS_GET_KEY_AT_(int64_t * index, char * key, int key_len) { ffncs_get_key_at(index, key, key_len); }
void fncs_get_key_at__(int64_t * index, char * key, int key_len) { ffncs_get_key_at(index, key, key_len); }
void FNCS_GET_KEY_AT__(int64_t * index, char * key, int key_len) { ffncs_get_key_at(index, key, key_len); }

static inline void ffncs_get_name(char * name, int name_len)
{
    string cxx_name = fncs::get_name();
    if (cxx_name.size() > name_len) {
        LERROR << "FORTRAN: fncs_get_name name(len=" << cxx_name.size()
            << ") does not fit into len=" << name_len << " character array";
        fncs::die();
        return;
    }
    copy_cxx_string_to_f(cxx_name, name, name_len);
}
void fncs_get_name_(char * name, int name_len) { ffncs_get_name(name, name_len); }
void FNCS_GET_NAME_(char * name, int name_len) { ffncs_get_name(name, name_len); }
void fncs_get_name__(char * name, int name_len) { ffncs_get_name(name, name_len); }
void FNCS_GET_NAME__(char * name, int name_len) { ffncs_get_name(name, name_len); }

static inline int ffncs_get_id()
{
    return fncs::get_id();
}
int fncs_get_id_() { return ffncs_get_id(); }
int FNCS_GET_ID_() { return ffncs_get_id(); }
int fncs_get_id__() { return ffncs_get_id(); }
int FNCS_GET_ID__() { return ffncs_get_id(); }

static inline int ffncs_get_simulator_count()
{
    return fncs::get_simulator_count();
}
int fncs_get_simulator_count_() { return ffncs_get_simulator_count(); }
int FNCS_GET_SIMULATOR_COUNT_() { return ffncs_get_simulator_count(); }
int fncs_get_simulator_count__() { return ffncs_get_simulator_count(); }
int FNCS_GET_SIMULATOR_COUNT__() { return ffncs_get_simulator_count(); }

static inline void ffncs_get_version(int * major, int * minor, int * patch)
{
    *major = FNCS_VERSION_MAJOR;
    *minor = FNCS_VERSION_MINOR;
    *patch = FNCS_VERSION_PATCH;
}
void fncs_get_version_(int * major, int * minor, int * patch) { ffncs_get_version(major, minor, patch); }
void FNCS_GET_VERSION_(int * major, int * minor, int * patch) { ffncs_get_version(major, minor, patch); }
void fncs_get_version__(int * major, int * minor, int * patch) { ffncs_get_version(major, minor, patch); }
void FNCS_GET_VERSION__(int * major, int * minor, int * patch) { ffncs_get_version(major, minor, patch); }

