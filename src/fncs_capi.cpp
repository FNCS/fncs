#include <stdlib.h>
#include <string.h>

#include <fncs.hpp>
#include <fncs.h>

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

static char * convert(const std::string & the_string)
{
   char *the_char = (char*)malloc(sizeof(char)*(the_string.size()+1));
   strcpy(the_char, the_string.c_str());
   return the_char; 
}

void fncs_get_events(char*** events, size_t *size)
{
    vector<string> events_vector = fncs::get_events();

    if (events_vector.empty()) {
        *events = NULL;
        *size = 0;
    }
    else {
        *events = (char**)malloc(sizeof(char*)*events_vector.size());
        for (size_t i=0; i<events_vector.size(); ++i) {
            (*events)[i] = convert(events_vector[i]);
        }
        *size = events_vector.size();
    }
}

char * fncs_get_value(const char *key)
{
    string value = fncs::get_value(key);
    if (value.empty()) {
        return NULL;
    }
    else {
        return convert(value);
    }
}

void fncs_get_values(
            const char *key,
            char *** values,
            size_t *size)
{
    vector<string> values_vector = fncs::get_values(key);

    if (values_vector.empty()) {
        *values = NULL;
        *size = 0;
    }
    else {
        *values = (char**)malloc(sizeof(char*)*values_vector.size());
        for (size_t i=0; i<values_vector.size(); ++i) {
            (*values)[i] = convert(values_vector[i]);
        }
        *size = values_vector.size();
    }
}

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

const char * fncs_get_name()
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

