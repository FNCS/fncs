#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include <fncs.hpp>
#include <fncs.h>

using namespace std;

void fncs_initialize()
{
    fncs::initialize();
}

void fncs_initialize_config(const char *configuration)
{
    fncs::initialize(configuration);
}

void fncs_agentRegister()
{
    fncs::agentRegister();
}

void fncs_agentRegisterConfig(const char *configuration)
{
    fncs::agentRegister(configuration);
}

int fncs_is_initialized()
{
    return fncs::is_initialized() ? 1 : 0;
}

fncs_time fncs_time_request(fncs_time next)
{
    return fncs::time_request(next);
}

void fncs_publish(const char *key, const char *value)
{
    fncs::publish(key, value);
}

void fncs_publish_anon(const char *key, const char *value)
{
    fncs::publish_anon(key, value);
}

void fncs_agentPublish(const char *value)
{
    fncs::agentPublish(value);
}

void fncs_route(
            const char *from,
            const char *to,
            const char *key,
            const char *value)
{
    fncs::route(from, to, key, value);
}

void fncs_die()
{
    fncs::die();
}

void fncs_finalize()
{
    fncs::finalize();
}

void fncs_update_time_delta(fncs_time delta)
{
    fncs::update_time_delta(delta);
}

static const char* convert(const string & the_string)
{
    return the_string.c_str();
}

static const char** convert(const vector<string> & the_values)
{
    const char **values = NULL;

    if (!the_values.empty()) {
        size_t size = the_values.size();
        values = (const char**)malloc(sizeof(const char*)*(size+1));
        for (size_t i=0; i<size; ++i) {
            values[i] = convert(the_values[i]);
        }
        values[size] = NULL; /* sentinal */
    }

    return values;
}

size_t fncs_get_events_size()
{
    return fncs::get_events().size();
}

const char** fncs_get_events()
{
    return convert(fncs::get_events());
}

char* fncs_agentGetEvents()
{
    return convert(fncs::agentGetEvents());
}

const char* fncs_get_event_at(size_t index)
{
    return convert(fncs::get_events()[index]);
}

const char* fncs_get_value(const char *key)
{
    return convert(fncs::get_value(key));
}

size_t fncs_get_values_size(const char *key)
{
    return fncs::get_values(key).size();
}

const char** fncs_get_values(const char *key)
{
    return convert(fncs::get_values(key));
}

const char* fncs_get_value_at(const char *key, size_t index)
{
    return convert(fncs::get_values(key)[index]);
}

size_t fncs_get_keys_size()
{
    return fncs::get_keys().size();
}

const char** fncs_get_keys()
{
    return convert(fncs::get_keys());
}

const char* fncs_get_key_at(size_t index)
{
    return convert(fncs::get_keys()[index]);
}

const char* fncs_get_name()
{
    return fncs::get_name().c_str();
}

int fncs_get_id()
{
    return fncs::get_id();
}

int fncs_get_simulator_count()
{
    return fncs::get_simulator_count();
}

void fncs_get_version(int *major, int *minor, int *patch)
{
    *major = FNCS_VERSION_MAJOR;
    *minor = FNCS_VERSION_MINOR;
    *patch = FNCS_VERSION_PATCH;
}

void _fncs_free(void * ptr)
{
    if (NULL != ptr) free(ptr);
}

