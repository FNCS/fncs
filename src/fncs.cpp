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


static bool is_initialized_ = false;
static bool die_is_fatal = false;
static string simulation_name = "";
static int simulation_id = 0;
static int n_sims = 0;
static fncs::time time_delta_multiplier = 0;
static fncs::time time_delta = 0;
static zsock_t *client = NULL;
static map<string,string> cache;
static vector<string> events;
static Echo echo;

#define TRACE echo << "TRACE: "
#define WARN  echo << "INFO:  "
#define FATAL echo << "FATAL: "

typedef map<string,vector<string> > clist_t;
static clist_t cache_list;

typedef map<string,fncs::Subscription> sub_string_t;
static sub_string_t subs_string;


void fncs::start_logging(Echo &echo)
{
    const char *fncs_log_filename = NULL;
    const char *fncs_log_stdout = NULL;
    const char *fncs_log_file = NULL;
    string simlog = simulation_name + ".log";

    /* name for fncs log file from environment */
    fncs_log_filename = getenv("FNCS_LOG_FILENAME");
    if (!fncs_log_filename) {
        if (simulation_name.empty()) {
            /* assume it's the broker */
            fncs_log_filename = "fncs_broker.log";
        }
        else {
            fncs_log_filename = simlog.c_str();
        }
    }

    /* whether to echo to stdout from environment */
    fncs_log_stdout = getenv("FNCS_LOG_STDOUT");
    if (!fncs_log_stdout) {
        fncs_log_stdout = "no";
    }

    /* whether to enable logging at all from environment */
    fncs_log_file = getenv("FNCS_LOG_FILE");
    if (!fncs_log_file) {
        fncs_log_file = "no";
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

    /* name for fncs config file from environment */
    fncs_config_file = getenv("FNCS_CONFIG_FILE");
    if (!fncs_config_file) {
        fncs_config_file = "fncs.zpl";
    }

    /* open and parse fncs configuration */
    config = zconfig_load(fncs_config_file);
    if (!config) {
        cerr << "could not open " << fncs_config_file << endl;
    }
    else {
        initialize(config);
        zconfig_destroy(&config);
    }
}


/* This version of initialize() reads configuration parameters directly
 * from the given string. */
void fncs::initialize(const string &configuration)
{
    zchunk_t *zchunk = NULL;
    zconfig_t *config = NULL;

    /* create a zchunk for parsing */
    zchunk = zchunk_new(configuration.c_str(), configuration.size());

    /* open and parse fncs configuration */
    config = zconfig_chunk_load(zchunk);
    if (!config) {
        cerr << "could not load configuration chunk" << endl;
    }
    else {
        zchunk_destroy(&zchunk);
        initialize(config);
        zconfig_destroy(&config);
    }
}


void fncs::initialize(zconfig_t *config)
{
    const char *fatal = NULL;
    const char *name = NULL;
    const char *broker_endpoint = NULL;
    const char *time_delta_string = NULL;
    int rc;
    zmsg_t *msg = NULL;
    zchunk_t *zchunk = NULL;
    zconfig_t *config_values = NULL;

    /* name from env var is tried first */
    name = getenv("FNCS_NAME");
    if (!name) {
        /* read sim name from config */
        name = zconfig_resolve(config, "/name", NULL);
        if (!name) {
            cerr << "FNCS_NAME env var not set and" << endl;
            cerr << "fncs config does not contain 'name'" << endl;
            die();
            return;
        }
    }
    else {
        /* put what we got from env var so it sends on to broker */
        zconfig_put(config, "/name", name);
    }
    simulation_name = name;

    fncs::start_logging(echo);

    /* whether die() should exit() */
    fatal = getenv("FNCS_FATAL");
    if (!fatal) {
        fatal =  "no";
    }
    if (fatal[0] == 'N'
            || fatal[0] == 'n'
            || fatal[0] == 'F'
            || fatal[0] == 'f') {
        TRACE << "fncs::die() will not call exit()" << endl;
        die_is_fatal = false;
    }
    else {
        TRACE << "fncs::die() will call exit(EXIT_FAILURE)" << endl;
        die_is_fatal = true;
    }

    /* broker location from env var is tried first */
    broker_endpoint = getenv("FNCS_BROKER");
    if (!broker_endpoint) {
        /* read broker location from config */
        broker_endpoint = zconfig_resolve(config, "/broker", NULL);
        if (!broker_endpoint) {
            TRACE << "fncs config does not contain 'broker'" << endl;
            TRACE << "broker default is tcp://localhost:5570" << endl;
            broker_endpoint = "tcp://localhost:5570";
        }
    }
    else {
        TRACE << "FNCS_BROKER env var sets the broker endpoint location" << endl;
        /* don't need to send broker address to broker */
        /*zconfig_put(config, "/broker", broker_endpoint);*/
    }
    TRACE << "broker = " << broker_endpoint << endl;

    /* time_delta from env var is tried first */
    time_delta_string = getenv("FNCS_TIME_DELTA");
    if (!time_delta_string) {
        /* read time delta from config */
        time_delta_string = zconfig_resolve(config, "/time_delta", NULL);
        if (!time_delta_string) {
            TRACE << "fncs config does not contain 'time_delta'" << endl;
            TRACE << "time_delta default is 1s" << endl;
            time_delta_string = "1s";
        }
    }
    else {
        TRACE << "FNCS_TIME_DELTA env var sets the time_delta_string" << endl;
        /* put what we got from env var so it sends on to broker */
        zconfig_put(config, "/time_delta", time_delta_string);
    }
    TRACE << "time_delta_string = " << time_delta_string << endl;
    time_delta = parse_time(time_delta_string);
    TRACE << "time_delta = " << time_delta << endl;
    time_delta_multiplier = time_unit_to_multiplier(time_delta_string);
    TRACE << "time_delta_multiplier = " << time_delta_multiplier << endl;

    /* parse subscriptions */
    config_values = zconfig_locate(config, "/values");
    if (config_values) {
        vector<fncs::Subscription> subs =
            fncs::parse_values(config_values);
        for (size_t i=0; i<subs.size(); ++i) {
            subs_string.insert(make_pair(subs[i].topic, subs[i]));
            TRACE << "initializing cache for '" << subs[i].key << "'='"
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
            TRACE << "'values' appears in config but no subscriptions" << endl;
        }
    }
    else {
        TRACE << "no subscriptions" << endl;
    }

    /* create zmq context and client socket */
    client = zsock_new(ZMQ_DEALER);
    if (!client) {
        FATAL << "socket creation failed" << endl;
        die();
        return;
    }
    if (!(zsock_resolve(client) != client)) {
        FATAL << "socket failed to resolve" << endl;
        die();
        return;
    }
    /* set client identity */
    rc = zmq_setsockopt(zsock_resolve(client), ZMQ_IDENTITY, name, strlen(name));
    if (rc) {
        FATAL << "socket identity failed" << endl;
        die();
        return;
    }
    /* finally connect to broker */
    rc = zsock_attach(client, broker_endpoint, false);
    if (rc) {
        FATAL << "socket connection to broker failed" << endl;
        die();
        return;
    }

    /* construct HELLO message; entire config goes with it */
    msg = zmsg_new();
    if (!msg) {
        FATAL << "could not construct HELLO message" << endl;
        die();
        return;
    }
    rc = zmsg_addstr(msg, HELLO);
    if (rc) {
        FATAL << "failed to append HELLO to message" << endl;
        die();
        return;
    }
    zchunk = zconfig_chunk_save(config);
    if (!zchunk) {
        FATAL << "failed to save config for HELLO message" << endl;
        die();
        return;
    }
    rc = zmsg_addmem(msg, zchunk_data(zchunk), zchunk_size(zchunk));
    if (rc) {
        FATAL << "failed to add config to HELLO message" << endl;
        die();
        return;
    }
    zchunk_destroy(&zchunk);
    TRACE << "sending HELLO" << endl;
    rc = zmsg_send(&msg, client);
    if (rc) {
        FATAL << "failed to send HELLO message" << endl;
        die();
        return;
    }

    /* receive ack */
    msg = zmsg_recv(client);
    if (!msg) {
        FATAL << "null message received" << endl;
        die();
        return;
    }
    /* first frame is type identifier */
    zframe_t *frame = zmsg_first(msg);
    if (!zframe_streq(frame, ACK)) {
        FATAL << "ACK expected, got " << frame << endl;
        die();
        return;
    }
    TRACE << "received ACK" << endl;
    /* next frame is connetion order ID */
    frame = zmsg_next(msg);
    if (!frame) {
        FATAL << "ACK message missing order ID" << endl;
        die();
        return;
    }
    simulation_id = atoi(fncs::to_string(frame).c_str());
    TRACE << "connection order ID is " << simulation_id << endl;

    /* next frame is n_sims */
    frame = zmsg_next(msg);
    if (!frame) {
        FATAL << "ACK message missing n_sims" << endl;
        die();
        return;
    }
    n_sims = atoi(fncs::to_string(frame).c_str());
    TRACE << "n_sims is " << n_sims << endl;

    zmsg_destroy(&msg);

    is_initialized_ = true;
}


bool fncs::is_initialized()
{
    return is_initialized_;
}


fncs::time fncs::time_request(fncs::time next)
{
    TRACE << "fncs::time_request(fncs::time)" << endl;

    if (!is_initialized_) {
        WARN << "fncs is not initialized" << endl;
        return next;
    }

    fncs::time granted;

    /* send TIME_REQUEST */
    TRACE << "sending TIME_REQUEST of " << next << " in sim units" << endl;
    next *= time_delta_multiplier;

    if (next % time_delta != 0) {
        FATAL << "time request "
            << next
            << " is not a multiple of time delta ("
            << time_delta
            << ")!" << endl;
        die();
        return next;
    }

    TRACE << "sending TIME_REQUEST of " << next << " nanoseconds" << endl;
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

    /* receive TIME_REQUEST and perhaps other message types */
    zmq_pollitem_t items[] = { { zsock_resolve(client), 0, ZMQ_POLLIN, 0 } };
    while (true) {
        int rc = 0;

        TRACE << "entering blocking poll" << endl;
        rc = zmq_poll(items, 1, -1);
        if (rc == -1) {
            FATAL << "client polling error: " << strerror(errno) << endl;
            die(); /* interrupted */
            return next;
        }

        if (items[0].revents & ZMQ_POLLIN) {
            zmsg_t *msg = NULL;
            zframe_t *frame = NULL;
            string message_type;

            TRACE << "incoming message" << endl;
            msg = zmsg_recv(client);
            if (!msg) {
                FATAL << "null message received" << endl;
                die();
                return next;
            }

            /* first frame is message type identifier */
            frame = zmsg_first(msg);
            if (!frame) {
                FATAL << "message missing type identifier" << endl;
                die();
                return next;
            }
            message_type = fncs::to_string(frame);

            /* dispatcher */
            if (fncs::TIME_REQUEST == message_type) {
                TRACE << "TIME_REQUEST received" << endl;

                /* next frame is time */
                frame = zmsg_next(msg);
                if (!frame) {
                    FATAL << "message missing time" << endl;
                    die();
                    return next;
                }
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

                TRACE << "PUBLISH received" << endl;

                /* next frame is topic */
                frame = zmsg_next(msg);
                if (!frame) {
                    FATAL << "message missing topic" << endl;
                    die();
                    return next;
                }
                topic = fncs::to_string(frame);

                /* next frame is value payload */
                frame = zmsg_next(msg);
                if (!frame) {
                    FATAL << "message missing value" << endl;
                    die();
                    return next;
                }
                value = fncs::to_string(frame);

                /* find cache short key */
                sub_str_itr = subs_string.find(topic);
                if (sub_str_itr != subs_string.end()) {
                    found = true;
                    subscription = sub_str_itr->second;
                }

                /* if found then store in cache */
                if (found) {
                    events.push_back(subscription.key);
                    if (subscription.is_list()) {
                        cache_list[subscription.key].push_back(value);
                        TRACE << "updated cache_list "
                            << "key='" << subscription.key << "' "
                            << "topic='" << topic << "' "
                            << "value='" << value << "' "
                            << "count=" << cache_list[subscription.key].size() << endl;
                    } else {
                        cache[subscription.key] = value;
                        TRACE << "updated cache "
                            << "key='" << subscription.key << "' "
                            << "topic='" << topic << "' "
                            << "value='" << value << "' " << endl;
                    }
                }
                else {
                    TRACE << "dropping PUBLISH message topic='"
                        << topic << "'" << endl;
                }
            }
            else {
                FATAL << "unrecognized message type" << endl;
                die();
                return next;
            }

            zmsg_destroy(&msg);
        }
    }

    TRACE << "granted " << granted << " nanoseonds" << endl;
    /* convert nanoseonds to sim's time unit */
    granted = convert_broker_to_sim_time(granted);
    TRACE << "granted " << granted << " in sim units" << endl;
    return granted;
}


void fncs::publish(const string &key, const string &value)
{
    TRACE << "fncs::publish(string,string)" << endl;

    if (!is_initialized_) {
        WARN << "fncs is not initialized" << endl;
        return;
    }

    string new_key = simulation_name + '/' + key;
    zstr_sendm(client, fncs::PUBLISH);
    zstr_sendm(client, new_key.c_str());
    zstr_send(client, value.c_str());
    TRACE << "sent PUBLISH '" << new_key << "'='" << value << "'" << endl;
}


void fncs::publish_anon(const string &key, const string &value)
{
    TRACE << "fncs::publish_anon(string,string)" << endl;

    if (!is_initialized_) {
        WARN << "fncs is not initialized" << endl;
        return;
    }

    zstr_sendm(client, fncs::PUBLISH);
    zstr_sendm(client, key.c_str());
    zstr_send(client, value.c_str());
    TRACE << "sent PUBLISH anon '" << key << "'='" << value << "'" << endl;
}


void fncs::route(
        const string &from,
        const string &to,
        const string &key,
        const string &value)
{
    TRACE << "fncs::route(string,string,string,string)" << endl;

    if (!is_initialized_) {
        WARN << "fncs is not initialized" << endl;
        return;
    }

    string new_key = simulation_name + '/' + from + '@' + to + '/' + key;
    zstr_sendm(client, fncs::PUBLISH);
    zstr_sendm(client, new_key.c_str());
    zstr_send(client, value.c_str());
    TRACE << "sent PUBLISH '" << new_key << "'='" << value << "'" << endl;
}


void fncs::die()
{
    TRACE << "fncs::die()" << endl;

    if (!is_initialized_) {
        WARN << "fncs is not initialized" << endl;
    }

    if (client) {
        zstr_send(client, fncs::DIE);
        zsock_destroy(&client);
    }

    is_initialized_ = false;

    if (die_is_fatal) {
        exit(EXIT_FAILURE);
    }
}


void fncs::finalize()
{
    TRACE << "fncs::finalize()" << endl;

    if (!is_initialized_) {
        WARN << "fncs is not initialized" << endl;
        return;
    }

    zmsg_t *msg = NULL;
    zframe_t *frame = NULL;

    zstr_send(client, fncs::BYE);

    /* receive BYE back */
    msg = zmsg_recv(client);
    if (!msg) {
        FATAL << "null message received" << endl;
        die();
        return;
    }

    /* first frame is type identifier */
    frame = zmsg_first(msg);
    if (!zframe_streq(frame, BYE)) {
        FATAL << "BYE expected, got " << frame << endl;
        die();
        return;
    }
    TRACE << "received BYE" << endl;

    zmsg_destroy(&msg);

    zsock_destroy(&client);
}


void fncs::update_time_delta(fncs::time delta)
{
    TRACE << "fncs::update_time_delta(fncs::time)" << endl;

    if (!is_initialized_) {
        WARN << "fncs is not initialized" << endl;
        return;
    }

    /* send TIME_DELTA */
    TRACE << "sending TIME_DELTA of " << delta << " in sim units" << endl;
    delta *= time_delta_multiplier;
    TRACE << "sending TIME_DELTA of " << delta << " nanoseconds" << endl;
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
    TRACE << "fncs::time_unit_to_multiplier(string)" << endl;

    fncs::time retval; 
    fncs::time ignore; 
    string unit;
    istringstream iss(value);

    iss >> ignore;
    if (!iss) {
        FATAL << "could not parse time value" << endl;
        die();
        return retval;
    }
    iss >> unit;
    if (!iss) {
        FATAL << "could not parse time unit" << endl;
        die();
        return retval;
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
        FATAL << "unrecognized time unit '" << unit << "'" << endl;
        die();
        return retval;
    }

    return retval;
}


fncs::time fncs::parse_time(const string &value)
{
    TRACE << "fncs::parse_time(string)" << endl;

    fncs::time retval; 
    string unit;
    istringstream iss(value);

    iss >> retval;
    if (!iss) {
        FATAL << "could not parse time value" << endl;
        die();
        return retval;
    }

    retval *= fncs::time_unit_to_multiplier(value);

    return retval;
}


fncs::Subscription fncs::parse_value(zconfig_t *config)
{
    TRACE << "fncs::parse_value(zconfig_t*)" << endl;

    /* a "value" block for a FNCS subscription looks like this
    foo [ = some_topic ]    # lookup key, optional topic
        topic = some_topic  # required iff topic did not appear earlier
        default = 10        # optional; default value
        type = int          # optional; currently unused; data type
        list = false        # optional; defaults to "false"
    */

    fncs::Subscription sub;
    const char *value = NULL;

    sub.key = zconfig_name(config);
    TRACE << "parsing value with key '" << sub.key << "'" << endl;

    /* check for topic attached to short key */
    value = zconfig_value(config);
    if (!value || 0 == strlen(value)) {
        TRACE << "key did not have topic attached" << endl;
        /* check for a 'topic' subheading */
        value = zconfig_resolve(config, "topic", NULL);
    }
    if (!value || 0 == strlen(value)) {
        TRACE << "key did not have 'topic' subheading" << endl;
        /* default is to use short key as subscription */
        sub.topic = sub.key;
    }
    else {
        sub.topic = value;
    }
    TRACE << "parsing key '" << sub.key << "' topic '" << sub.topic << "'" << endl;

    value = zconfig_resolve(config, "default", NULL);
    if (!value) {
        TRACE << "parsing value '" << sub.key << "', missing 'default'" << endl;
    }
    sub.def = value? value : "";

    value = zconfig_resolve(config, "type", NULL);
    if (!value) {
        TRACE << "parsing value '" << sub.key << "', missing 'type'" << endl;
    }
    sub.type = value? value : "double";

    value = zconfig_resolve(config, "list", NULL);
    if (!value) {
        TRACE << "parsing value '" << sub.key << "', missing 'list'" << endl;
    }
    sub.list = value? value : "false";

    return sub;
}


vector<fncs::Subscription> fncs::parse_values(zconfig_t *config)
{
    TRACE << "fncs::parse_values(zconfig_t*)" << endl;

    vector<fncs::Subscription> subs;
    string name;
    zconfig_t *child = NULL;

    name = zconfig_name(config);
    if (name != "values") {
        FATAL << "error parsing 'values', wrong config object '" << name << "'" << endl;
        die();
        return subs;
    }

    child = zconfig_child(config);
    while (child) {
        subs.push_back(parse_value(child));
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
    TRACE << "fncs::get_events() [" << events.size() << "]" << endl;

    if (!is_initialized_) {
        WARN << "fncs is not initialized" << endl;
        return vector<string>();
    }

    return events;
}


string fncs::get_value(const string &key)
{
    TRACE << "fncs::get_value(" << key << ")" << endl;

    if (!is_initialized_) {
        WARN << "fncs is not initialized" << endl;
        return "";
    }

    if (0 == cache.count(key)) {
        FATAL << "key '" << key << "' not found in cache" << endl;
        die();
        return "";
    }

    return cache[key];
}


vector<string> fncs::get_values(const string &key)
{
    TRACE << "fncs::get_values(" << key << ")" << endl;

    if (!is_initialized_) {
        WARN << "fncs is not initialized" << endl;
        return vector<string>();
    }

    vector<string> values;

    if (0 == cache_list.count(key)) {
        FATAL << "key '" << key << "' not found in cache list" << endl;
        die();
        return values;
    }

    values = cache_list[key];
    TRACE << "key '" << key << "' has " << values.size() << " values" << endl;
    return values;
}


string fncs::get_name()
{
    return simulation_name;
}


fncs::time fncs::get_time_delta()
{
    return time_delta;
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

