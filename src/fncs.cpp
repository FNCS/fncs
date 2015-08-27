/* autoconf header */
#include "config.h"

/* C++  standard headers */
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <functional>
#include <map>
#include <sstream>
#include <string>
#include <vector>

/* for fncs::timer() */
#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#elif defined(__FreeBSD__)
#include <time.h>
#elif defined(_WIN32)
//#include <windows.h>
#else
#include <time.h>
#endif
#include <assert.h>

/* 3rd party headers */
#include "czmq.h"

/* fncs headers */
#include "echo.hpp"
#include "fncs.hpp"
#include "fncs_internal.hpp"

using namespace ::std;
using fncs::Echo;


static string simulation_name = "";
static int simulation_id = 0;
static int n_sims = 0;
static fncs::time time_delta_multiplier = 0;
static fncs::time time_delta = 0;
static zsock_t *client = NULL;
static map<string,string> cache;
static vector<string> events;
static Echo echo;

typedef map<string,vector<string> > clist_t;
static clist_t cache_list;

typedef map<string,vector<pair<string,string> > > mlist_t;
static mlist_t match_list;

typedef map<string,fncs::Subscription> sub_string_t;
static sub_string_t subs_string;

typedef map<zrex_t*,fncs::Subscription> sub_zrex_t;
static sub_zrex_t subs_zrex;


void fncs::start_logging(Echo &echo)
{
    const char *fncs_log_filename = NULL;
    const char *fncs_log_stdout = NULL;
    const char *fncs_log_file = NULL;

    /* name for fncs log file from environment */
    fncs_log_filename = getenv("FNCS_LOG_FILENAME");
    if (!fncs_log_filename) {
        fncs_log_filename = "fncs.log";
    }

    /* whether to echo to stdout from environment */
    fncs_log_stdout = getenv("FNCS_LOG_STDOUT");
    if (!fncs_log_stdout) {
        fncs_log_stdout = "yes";
    }

    /* whether to enable logging at all from environment */
    fncs_log_file = getenv("FNCS_LOG_FILE");
    if (!fncs_log_file) {
        fncs_log_file = "yes";
    }

    /* start our logger */
    if (fncs_log_stdout[0] == 'N'
            || fncs_log_stdout[0] == 'n'
            || fncs_log_stdout[0] == 'F'
            || fncs_log_stdout[0] == 'f') {
    }
    else {
        echo.enable_stdout();
    }

    if (fncs_log_file[0] == 'N'
            || fncs_log_file[0] == 'n'
            || fncs_log_file[0] == 'F'
            || fncs_log_file[0] == 'f') {
    }
    else {
        echo.open(fncs_log_filename);
    }
}


/* This version of initialize() checks for a config filename in the
 * environment and then defaults to a known name. */
void fncs::initialize()
{
    const char *fncs_config_file = NULL;
    zconfig_t *config = NULL;

    start_logging(echo);

    echo << "fncs::initialize()" << endl;

    /* name for fncs config file from environment */
    fncs_config_file = getenv("FNCS_CONFIG_FILE");
    if (!fncs_config_file) {
        fncs_config_file = "fncs.zpl";
    }
    echo << "FNCS_CONFIG_FILE: " << fncs_config_file << endl;

    /* open and parse fncs configuration */
    config = zconfig_load(fncs_config_file);
    if (!config) {
        echo << "could not open " << fncs_config_file << endl;
        die();
    }

    initialize(config);
    zconfig_destroy(&config);
}


/* This version of initialize() reads configuration parameters directly
 * from the given string. */
void fncs::initialize(const string &configuration)
{
    zchunk_t *zchunk = NULL;
    zconfig_t *config = NULL;

    start_logging(echo);

    echo << "fncs::initialize(string)" << endl;

    /* create a zchunk for parsing */
    zchunk = zchunk_new(configuration.c_str(), configuration.size());

    /* open and parse fncs configuration */
    config = zconfig_chunk_load(zchunk);
    if (!config) {
        echo << "could not load configuration chunk" << endl;
        die();
    }
    zchunk_destroy(&zchunk);

    initialize(config);
    zconfig_destroy(&config);
}


void fncs::initialize(zconfig_t *config)
{
    const char *name = NULL;
    const char *broker_endpoint = NULL;
    const char *time_delta_string = NULL;
    int rc;
    zmsg_t *msg = NULL;
    zchunk_t *zchunk = NULL;
    zconfig_t *config_values = NULL;

    echo << "fncs::initialize(zconfig_t*)" << endl;

    /* name from env var is tried first */
    name = getenv("FNCS_NAME");
    if (!name) {
        /* read sim name from config */
        name = zconfig_resolve(config, "/name", NULL);
        if (!name) {
            echo << "FNCS_NAME env var not set and" << endl;
            echo << "fncs config does not contain 'name'" << endl;
            die();
        }
    }
    else {
        echo << "FNCS_NAME env var sets the name" << endl;
        /* put what we got from env var so it sends on to broker */
        zconfig_put(config, "/name", name);
    }
    simulation_name = name;
    echo << "name = '" << name << "'" << endl;

    /* broker location from env var is tried first */
    broker_endpoint = getenv("FNCS_BROKER");
    if (!broker_endpoint) {
        /* read broker location from config */
        broker_endpoint = zconfig_resolve(config, "/broker", NULL);
        if (!broker_endpoint) {
            echo << "fncs config does not contain 'broker'" << endl;
            broker_endpoint = "tcp://localhost:5570";
        }
    }
    else {
        echo << "FNCS_BROKER env var sets the broker endpoint location" << endl;
        /* don't need to send broker address to broker */
        /*zconfig_put(config, "/broker", broker_endpoint);*/
    }
    echo << "broker = " << broker_endpoint << endl;

    /* time_delta from env var is tried first */
    time_delta_string = getenv("FNCS_TIME_DELTA");
    if (!time_delta_string) {
        /* read time delta from config */
        time_delta_string = zconfig_resolve(config, "/time_delta", NULL);
        if (!time_delta_string) {
            echo << "fncs config does not contain 'time_delta'" << endl;
            die();
        }
    }
    else {
        echo << "FNCS_TIME_DELTA env var sets the time_delta_string" << endl;
        /* put what we got from env var so it sends on to broker */
        zconfig_put(config, "/time_delta", time_delta_string);
    }
    echo << "time_delta_string = " << time_delta_string << endl;
    time_delta = parse_time(time_delta_string);
    echo << "time_delta = " << time_delta << endl;
    time_delta_multiplier = time_unit_to_multiplier(time_delta_string);
    echo << "time_delta_multiplier = " << time_delta_multiplier << endl;

    /* parse subscriptions */
    config_values = zconfig_locate(config, "/values");
    if (config_values) {
        vector<fncs::Subscription> subs =
            fncs::parse_values(config_values);
        for (size_t i=0; i<subs.size(); ++i) {
            subs_string.insert(make_pair(subs[i].topic, subs[i]));
            echo << "initializing cache for '" << subs[i].key << "'='"
                << subs[i].def << "'" << endl;
            if (subs[i].is_list()) {
                if (subs[i].def.empty()) {
                    cache_list[subs[i].key] = vector<string>();
                }
                else {
                    cache_list[subs[i].key] = vector<string>(1, subs[i].def);
                }
            }
            else {
                cache[subs[i].key] = subs[i].def;
            }
        }
        if (subs.empty()) {
            echo << "'values' appears in config but no subscriptions" << endl;
        }
    }
    else {
        echo << "no subscriptions" << endl;
    }
    config_values = zconfig_locate(config, "/matches");
    if (config_values) {
        vector<fncs::Subscription> subs =
            fncs::parse_matches(config_values);
        for (size_t i=0; i<subs.size(); ++i) {
            echo << "compiling re'" << subs[i].topic << "'" << endl;
            subs_zrex.insert(make_pair(
                        zrex_new(subs[i].topic.c_str()),
                        subs[i]));
            echo << "initializing match cache for '" << subs[i].key << "'" << endl;
            match_list[subs[i].key] = vector<pair<string,string> >();
        }
        if (subs.empty()) {
            echo << "'matches' appears in config but no subscriptions" << endl;
        }
    }
    else {
        echo << "no subscriptions" << endl;
    }

    /* create zmq context and client socket */
    client = zsock_new(ZMQ_DEALER);
    if (!client) {
        echo << "socket creation failed" << endl;
        die();
    }
    if (!(zsock_resolve(client) != client)) {
        echo << "socket failed to resolve" << endl;
        die();
    }
    /* set client identity */
    rc = zmq_setsockopt(zsock_resolve(client), ZMQ_IDENTITY, name, strlen(name));
    if (rc) {
        echo << "socket identity failed" << endl;
        die();
    }
    /* finally connect to broker */
    rc = zsock_attach(client, broker_endpoint, false);
    if (rc) {
        echo << "socket connection to broker failed" << endl;
        die();
    }

    /* construct HELLO message; entire config goes with it */
    msg = zmsg_new();
    if (!msg) {
        echo << "could not construct HELLO message" << endl;
        die();
    }
    rc = zmsg_addstr(msg, HELLO);
    if (rc) {
        echo << "failed to append HELLO to message" << endl;
        die();
    }
    zchunk = zconfig_chunk_save(config);
    if (!zchunk) {
        echo << "failed to save config for HELLO message" << endl;
        die();
    }
    rc = zmsg_addmem(msg, zchunk_data(zchunk), zchunk_size(zchunk));
    if (rc) {
        echo << "failed to add config to HELLO message" << endl;
        die();
    }
    zchunk_destroy(&zchunk);
    echo << "sending HELLO" << endl;
    rc = zmsg_send(&msg, client);
    if (rc) {
        echo << "failed to send HELLO message" << endl;
        die();
    }

    /* receive ack */
    msg = zmsg_recv(client);
    if (!msg) {
        echo << "null message received" << endl;
        die();
    }
    /* first frame is type identifier */
    zframe_t *frame = zmsg_first(msg);
    if (!zframe_streq(frame, ACK)) {
        echo << "ACK expected, got " << frame << endl;
        die();
    }
    echo << "received ACK" << endl;
    /* next frame is connetion order ID */
    frame = zmsg_next(msg);
    if (!frame) {
        echo << "ACK message missing order ID" << endl;
        die();
    }
    echo << frame << endl;
    simulation_id = atoi(fncs::to_string(frame).c_str());
    echo << "connection order ID is " << simulation_id << endl;

    /* next frame is n_sims */
    frame = zmsg_next(msg);
    if (!frame) {
        echo << "ACK message missing n_sims" << endl;
        die();
    }
    echo << frame << endl;
    n_sims = atoi(fncs::to_string(frame).c_str());
    echo << "n_sims is " << n_sims << endl;

    zmsg_destroy(&msg);
}


fncs::time fncs::time_request(fncs::time next)
{
    fncs::time granted;

    echo << "fncs::time_request(fncs::time)" << endl;

    /* send TIME_REQUEST */
    echo << "sending TIME_REQUEST of " << next << " in sim units" << endl;
    next *= time_delta_multiplier;
    echo << "sending TIME_REQUEST of " << next << " nanoseconds" << endl;
    zstr_sendm(client, fncs::TIME_REQUEST);
    zstr_sendf(client, "%llu", next);

    /* sending of the time request implies we are done with the cache
     * list, but the other cache remains as a last value cache */
    /* only clear the vectors associated with cache list keys because
     * the keys should remain valid i.e. empty lists are meaningful */
    events.clear();
    for (clist_t::iterator it=cache_list.begin(); it!=cache_list.end(); ++it) {
        it->second.clear();
    }
    for (mlist_t::iterator it=match_list.begin(); it!=match_list.end(); ++it) {
        it->second.clear();
    }

    /* receive TIME_REQUEST and perhaps other message types */
    zmq_pollitem_t items[] = { { zsock_resolve(client), 0, ZMQ_POLLIN, 0 } };
    while (true) {
        int rc = 0;

        echo << "entering blocking poll" << endl;
        rc = zmq_poll(items, 1, -1);
        if (rc == -1) {
            echo << "client polling error: " << strerror(errno) << endl;
            die(); /* interrupted */
        }

        if (items[0].revents & ZMQ_POLLIN) {
            zmsg_t *msg = NULL;
            zframe_t *frame = NULL;
            string message_type;

            echo << "incoming message" << endl;
            msg = zmsg_recv(client);
            if (!msg) {
                echo << "null message received" << endl;
                die();
            }

            /* first frame is message type identifier */
            frame = zmsg_first(msg);
            if (!frame) {
                echo << "message missing type identifier" << endl;
                die();
            }
            echo << frame << endl;
            message_type = fncs::to_string(frame);

            /* dispatcher */
            if (fncs::TIME_REQUEST == message_type) {
                echo << "TIME_REQUEST received" << endl;

                /* next frame is time */
                frame = zmsg_next(msg);
                if (!frame) {
                    echo << "message missing time" << endl;
                    die();
                }
                echo << frame << endl;
                /* convert time string to nanoseconds */
                {
                    istringstream iss(fncs::to_string(frame));
                    iss >> granted;
                }

                /* destroy message early since a returned TIME_REQUEST
                 * indicates we can move on with the break */
                zmsg_destroy(&msg);
                break;
            }
            else if (fncs::PUBLISH == message_type) {
                string topic;
                string value;
                sub_string_t::const_iterator sub_str_itr;
                fncs::Subscription subscription;
                bool found = false;

                echo << "PUBLISH received" << endl;

                /* next frame is topic */
                frame = zmsg_next(msg);
                if (!frame) {
                    echo << "message missing topic" << endl;
                    die();
                }
                echo << frame << endl;
                topic = fncs::to_string(frame);

                /* next frame is value payload */
                frame = zmsg_next(msg);
                if (!frame) {
                    echo << "message missing value" << endl;
                    die();
                }
                echo << frame << endl;
                value = fncs::to_string(frame);

                /* find cache short key */
                sub_str_itr = subs_string.find(topic);
                if (sub_str_itr != subs_string.end()) {
                    found = true;
                    subscription = sub_str_itr->second;
                }
                else {
                    for (sub_zrex_t::const_iterator it=subs_zrex.begin();
                            it!=subs_zrex.end(); ++it) {
                        if (zrex_matches(it->first, topic.c_str())) {
                            found = true;
                            subscription = it->second;
                        }
                    }
                }
                /* if found then store in cache */
                if (found) {
                    events.push_back(subscription.key);
                    if (subscription.is_match()) {
                        match_list[subscription.key].push_back(
                                make_pair(topic,value));
                        echo << "updated match_list "
                            << "key='" << subscription.key << "' "
                            << "topic='" << topic << "' "
                            << "value='" << value << "' "
                            << "count=" << match_list[subscription.key].size() << endl;
                    }
                    else if (subscription.is_list()) {
                        cache_list[subscription.key].push_back(value);
                        echo << "updated cache_list "
                            << "key='" << subscription.key << "' "
                            << "topic='" << topic << "' "
                            << "value='" << value << "' "
                            << "count=" << match_list[subscription.key].size() << endl;
                    } else {
                        cache[subscription.key] = value;
                        echo << "updated cache "
                            << "key='" << subscription.key << "' "
                            << "topic='" << topic << "' "
                            << "value='" << value << "' " << endl;
                    }
                }
                else {
                    echo << "dropping PUBLISH message topic='"
                        << topic << "'" << endl;
                }
            }
            else {
                echo << "unrecognized message type" << endl;
                die();
            }

            zmsg_destroy(&msg);
        }
    }

    echo << "granted " << granted << " nanoseonds" << endl;
    /* convert nanoseonds to sim's time unit */
    granted = convert_broker_to_sim_time(granted);
    echo << "granted " << granted << " in sim units" << endl;
    return granted;
}


void fncs::publish(const string &key, const string &value)
{
    string new_key = simulation_name + '/' + key;
    echo << "fncs::publish(string,string)" << endl;
    zstr_sendm(client, fncs::PUBLISH);
    zstr_sendm(client, new_key.c_str());
    zstr_send(client, value.c_str());
    echo << "sent PUBLISH '" << new_key << "'='" << value << "'" << endl;
}


void fncs::publish_anon(const string &key, const string &value)
{
    echo << "fncs::publish_anon(string,string)" << endl;
    zstr_sendm(client, fncs::PUBLISH);
    zstr_sendm(client, key.c_str());
    zstr_send(client, value.c_str());
    echo << "sent PUBLISH anon '" << key << "'='" << value << "'" << endl;
}


void fncs::route(
        const string &from,
        const string &to,
        const string &key,
        const string &value)
{
    string new_key = simulation_name + '/' + from + ':' + to + '/' + key;
    echo << "fncs::route(string,string,string,string)" << endl;
    zstr_sendm(client, fncs::PUBLISH);
    zstr_sendm(client, new_key.c_str());
    zstr_send(client, value.c_str());
    echo << "sent PUBLISH '" << new_key << "'='" << value << "'" << endl;
}


void fncs::die()
{
    echo << "fncs::die()" << endl;
    if (client) {
        zstr_send(client, fncs::DIE);
        zsock_destroy(&client);
    }
    exit(EXIT_FAILURE);
}


void fncs::finalize()
{
    zmsg_t *msg = NULL;
    zframe_t *frame = NULL;

    echo << "fncs::finalize()" << endl;

    zstr_send(client, fncs::BYE);

    /* receive BYE back */
    msg = zmsg_recv(client);
    if (!msg) {
        echo << "null message received" << endl;
        die();
    }

    /* first frame is type identifier */
    frame = zmsg_first(msg);
    if (!zframe_streq(frame, BYE)) {
        echo << "BYE expected, got " << frame << endl;
        die();
    }
    echo << "received BYE" << endl;

    zmsg_destroy(&msg);

    zsock_destroy(&client);
}


void fncs::update_time_delta(fncs::time delta)
{
    /* send TIME_DELTA */
    echo << "sending TIME_DELTA of " << delta << " in sim units" << endl;
    delta *= time_delta_multiplier;
    echo << "sending TIME_DELTA of " << delta << " nanoseconds" << endl;
    zstr_sendm(client, fncs::TIME_DELTA);
    zstr_sendf(client, "%llu", delta);
}


ostream& operator << (ostream& os, zframe_t *self) {
    assert (self);
    assert (zframe_is (self));

    byte *data = zframe_data (self);
    size_t size = zframe_size (self);

    //  Probe data to check if it looks like unprintable binary
    int is_bin = 0;
    uint char_nbr;
    for (char_nbr = 0; char_nbr < size; char_nbr++)
        if (data [char_nbr] < 9 || data [char_nbr] > 127)
            is_bin = 1;

    size_t buffer_size = 31;
    buffer_size += is_bin? size*2 : size;
    char *buffer = (char*)zmalloc(buffer_size);
    snprintf (buffer, 30, "[%03d] ", (int) size);
    for (char_nbr = 0; char_nbr < size; char_nbr++) {
        if (is_bin)
            sprintf (buffer + strlen (buffer), "%02X", (unsigned char) data [char_nbr]);
        else
            sprintf (buffer + strlen (buffer), "%c", data [char_nbr]);
    }

    os << buffer;

    return os;
}


fncs::time fncs::time_unit_to_multiplier(const string &value)
{
    fncs::time retval; 
    fncs::time ignore; 
    string unit;
    istringstream iss(value);

    echo << "fncs::time_unit_to_multiplier(string)" << endl;

    iss >> ignore;
    if (!iss) {
        echo << "could not parse time value" << endl;
        die();
    }
    iss >> unit;
    if (!iss) {
        echo << "could not parse time unit" << endl;
        die();
    }

    if ("d" == unit
            || "day" == unit
            || "days" == unit) {
        retval = 24ULL * 60ULL * 60ULL * 1000000000ULL;
    }
    else if ("h" == unit
            || "hour" == unit
            || "hours" == unit) {
        retval = 60ULL * 60ULL * 1000000000ULL;
    }
    else if ("m" == unit
            || "min" == unit
            || "minute" == unit
            || "minutes" == unit) {
        retval = 60ULL * 1000000000ULL;
    }
    else if ("s" == unit
            || "sec" == unit
            || "second" == unit
            || "seconds" == unit) {
        retval = 1000000000ULL;
    }
    else if ("ms" == unit
            || "msec" == unit
            || "millisec" == unit
            || "millisecond" == unit
            || "milliseconds" == unit) {
        retval = 1000000ULL;
    }
    else if ("us" == unit
            || "usec" == unit
            || "microsec" == unit
            || "microsecond" == unit
            || "microseconds" == unit) {
        retval = 1000ULL;
    }
    else if ("ns" == unit
            || "nsec" == unit
            || "nanosec" == unit
            || "nanosecond" == unit
            || "nanoseconds" == unit) {
        retval = 1ULL;
    }
    else {
        echo << "unrecognized time unit '" << unit << "'" << endl;
        die();
    }

    return retval;
}


fncs::time fncs::parse_time(const string &value)
{
    fncs::time retval; 
    string unit;
    istringstream iss(value);

    echo << "fncs::parse_time(string)" << endl;

    iss >> retval;
    if (!iss) {
        echo << "could not parse time value" << endl;
        die();
    }

    retval *= fncs::time_unit_to_multiplier(value);

    return retval;
}


fncs::Subscription fncs::parse_value(zconfig_t *config)
{
    fncs::Subscription sub;
    const char *value = NULL;

    echo << "fncs::parse_value(zconfig_t*)" << endl;

    sub.key = zconfig_name(config);

    value = zconfig_resolve(config, "topic", NULL);
    if (!value) {
        echo << "error parsing value '" << sub.key << "', missing 'topic'" << endl;
        die();
    }
    sub.topic = value;

    value = zconfig_resolve(config, "default", NULL);
    if (!value) {
        echo << "parsing value '" << sub.key << "', missing 'default'" << endl;
    }
    sub.def = value? value : "";

    value = zconfig_resolve(config, "type", NULL);
    if (!value) {
        echo << "parsing value '" << sub.key << "', missing 'type'" << endl;
    }
    sub.type = value? value : "double";

    value = zconfig_resolve(config, "list", NULL);
    if (!value) {
        echo << "parsing value '" << sub.key << "', missing 'list'" << endl;
    }
    sub.list = value? value : "false";

    return sub;
}


fncs::Subscription fncs::parse_match(zconfig_t *config)
{
    fncs::Subscription sub;
    const char *value = NULL;

    echo << "fncs::parse_match(zconfig_t*)" << endl;

    sub.key = zconfig_name(config);
    sub.match = true;

    value = zconfig_resolve(config, "topic", NULL);
    if (!value) {
        echo << "error parsing value '" << sub.key << "', missing 'topic'" << endl;
        die();
    }
    sub.topic = value;

    return sub;
}


vector<fncs::Subscription> fncs::parse_values(zconfig_t *config)
{
    vector<fncs::Subscription> subs;
    string name;
    zconfig_t *child = NULL;

    echo << "fncs::parse_values(zconfig_t*)" << endl;

    name = zconfig_name(config);
    if (name != "values") {
        echo << "error parsing 'values', wrong config object '" << name << "'" << endl;
        die();
    }

    child = zconfig_child(config);
    while (child) {
        subs.push_back(parse_value(child));
        child = zconfig_next(child);
    }

    return subs;
}


vector<fncs::Subscription> fncs::parse_matches(zconfig_t *config)
{
    vector<fncs::Subscription> subs;
    string name;
    zconfig_t *child = NULL;

    echo << "fncs::parse_matches(zconfig_t*)" << endl;

    name = zconfig_name(config);
    if (name != "matches") {
        echo << "error parsing 'matches', wrong config object '" << name << "'" << endl;
        die();
    }

    child = zconfig_child(config);
    while (child) {
        subs.push_back(parse_match(child));
        child = zconfig_next(child);
    }

    return subs;
}


string fncs::to_string(zframe_t *frame)
{
    return string((const char *)zframe_data(frame), zframe_size(frame));
}


vector<string> fncs::get_events()
{
    echo << "fncs::get_events() [" << events.size() << "]" << endl;
    return events;
}

/* I don't think the following behavior is what is wanted. */
#if 0

string fncs::get_value(const string &key)
{
    if (0 == cache.count(key)) {
        echo << "key '" << key << "' not found in cache" << endl;
        if (0 == cache_list.count(key)) {
            echo << "key '" << key << "' not found in cache or cache list" << endl;
            die();
        }
        else {
            return get_values(key).back();
        }
    }
    return cache[key];
}


vector<string> fncs::get_values(const string &key)
{
    if (0 == cache_list.count(key)) {
        echo << "key '" << key << "' not found in cache list" << endl;
        if (0 == cache.count(key)) {
            echo << "key '" << key << "' not found in cache list or cache" << endl;
            die();
        }
        else {
            return vector<string>(1, get_value(key));
        }
    }
    return cache_list[key];
}

#else
/* This seems like clearer semantics. */

string fncs::get_value(const string &key)
{
    echo << "fncs::get_value(" << key << ")" << endl;
    if (0 == cache.count(key)) {
        echo << "key '" << key << "' not found in cache" << endl;
        die();
    }
    return cache[key];
}


vector<string> fncs::get_values(const string &key)
{
    vector<string> values;

    echo << "fncs::get_values(" << key << ")" << endl;

    if (0 == cache_list.count(key)) {
        echo << "key '" << key << "' not found in cache list" << endl;
        die();
    }

    values = cache_list[key];
    echo << "key '" << key << "' has " << values.size() << " values" << endl;
    return values;
}


vector<pair<string,string> > fncs::get_matches(const string &key)
{
    vector<pair<string,string> > values;

    echo << "fncs::get_matches(" << key << ")" << endl;

    if (0 == match_list.count(key)) {
        echo << "key '" << key << "' not found in match list" << endl;
        die();
    }

    values = match_list[key];
    echo << "key '" << key << "' has " << values.size() << " values" << endl;
    return values;
}
#endif


string fncs::get_name()
{
    return simulation_name;
}


int fncs::get_id()
{
    return simulation_id;
}


int fncs::get_simulator_count()
{
    return n_sims;
}


fncs::time fncs::convert_broker_to_sim_time(fncs::time value)
{
    return value / time_delta_multiplier;
}

double fncs::timer()
{
#ifdef __MACH__
    /* OS X does not have clock_gettime, use clock_get_time */
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    return (double)(mts.tv_sec) + (double)(mts.tv_nsec)/1000000000.0;
#elif defined(__FreeBSD__)
    struct timespec ts;
    /* Works on FreeBSD */
    long retval = clock_gettime(CLOCK_MONOTONIC, &ts);
    assert(0 == retval);
    return (double)(ts.tv_sec) + (double)(ts.tv_nsec)/1000000000.0;
#elif defined(_WIN32)
    static LARGE_INTEGER freq, start;
    LARGE_INTEGER count;
    BOOL retval;
    retval = QueryPerformanceCounter(&count);
    assert(retval);
    if (!freq.QuadPart) { // one time initialization
        retval = QueryPerformanceFrequency(&freq);
        assert(retval);
        start = count;
    }
    return (double)(count.QuadPart - start.QuadPart) / freq.QuadPart;
#else
    struct timespec ts;
    /* Works on Linux */
    long retval = clock_gettime(CLOCK_REALTIME, &ts);
    assert(0 == retval);
    return (double)(ts.tv_sec) + (double)(ts.tv_nsec)/1000000000.0;
#endif
}

