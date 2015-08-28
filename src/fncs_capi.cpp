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

fncs_time fncs_time_request(fncs_time next)
{
    return fncs::time_request(next);
}

void fncs_publish(const char *key, const char *value)
{
    fncs::publish(key, value);
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

static char* convert(const string & the_string)
{
    char *str = NULL;

    str = (char*)malloc(sizeof(char)*(the_string.size()+1));
    strcpy(str, the_string.c_str());

    return str; 
}

static char** convert(const vector<string> & the_values)
{
    char **values = NULL;

    if (!the_values.empty()) {
        size_t size = the_values.size();
        values = (char**)malloc(sizeof(char*)*(size+1));
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

char** fncs_get_events()
{
    return convert(fncs::get_events());
}

char* fncs_get_value(const char *key)
{
    return convert(fncs::get_value(key));
}

size_t fncs_get_values_size(const char *key)
{
    return fncs::get_values(key).size();
}

char** fncs_get_values(const char *key)
{
    return convert(fncs::get_values(key));
}

#if 0
void fncs_get_matches(
            const char *key,
            char *** topics,
            char *** values,
            size_t *size)
{
    vector<pair<string,string> > matches_vector = fncs::get_matches(key);

    if (matches_vector.empty()) {
        *topics = NULL;
        *values = NULL;
        *size = 0;
    }
    else {
        *topics = (char**)malloc(sizeof(char*)*matches_vector.size());
        *values = (char**)malloc(sizeof(char*)*matches_vector.size());
        for (size_t i=0; i<matches_vector.size(); ++i) {
            (*topics)[i] = convert(matches_vector[i].first);
            (*values)[i] = convert(matches_vector[i].second);
        }
        *size = matches_vector.size();
    }
}
#endif

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

