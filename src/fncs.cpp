/* autoconf header */
#include "config.h"

/* C++  standard headers */
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <functional>
#include <iostream>
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
#include "log.hpp"
#include "fncs.hpp"
#include "fncs_internal.hpp"

using namespace ::std;

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

typedef map<string,vector<string> > clist_t;
static clist_t cache_list;

typedef map<string,fncs::Subscription> sub_string_t;
static sub_string_t subs_string;


#if defined(_WIN32)
static void signal_handler_reset() { } /* no-op */
#else
#include <signal.h>
static struct sigaction sigint_default;
static struct sigaction sigterm_default;
static void signal_handler (int signal_value)
{
    zctx_interrupted = 1;
    zsys_interrupted = 1;
    if (SIGINT == signal_value && NULL != sigint_default.sa_handler) {
        sigint_default.sa_handler(signal_value);
    }
    if (SIGTERM == signal_value && NULL != sigterm_default.sa_handler) {
        sigterm_default.sa_handler(signal_value);
    }
}
static void signal_handler_reset()
{
    zsys_handler_set(NULL);
    sigaction(SIGINT, NULL, &sigint_default);
    sigaction(SIGTERM, NULL, &sigterm_default);
    zsys_handler_set(signal_handler);
}
#endif




void fncs::start_logging()
{
    const char *fncs_log_filename = NULL;
    const char *fncs_log_stdout = NULL;
    const char *fncs_log_file = NULL;
    const char *fncs_log_level = NULL;
    string simlog = simulation_name + ".log";
    bool log_file = false;
    bool log_stdout = false;

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

    /* whether to enable logging at all from environment */
    fncs_log_level = getenv("FNCS_LOG_LEVEL");
    if (!fncs_log_level) {
        fncs_log_level = "WARNING";
    }

    /* start our logger */
    if (fncs_log_stdout[0] == 'N'
            || fncs_log_stdout[0] == 'n'
            || fncs_log_stdout[0] == 'F'
            || fncs_log_stdout[0] == 'f') {
    }
    else {
        log_stdout = true;
    }

    if (fncs_log_file[0] == 'N'
            || fncs_log_file[0] == 'n'
            || fncs_log_file[0] == 'F'
            || fncs_log_file[0] == 'f') {
    }
    else {
        log_file = true;
    }

    if (log_stdout && log_file) {
        FILE *pFile = fopen(fncs_log_filename, "a");
        if (!pFile) {
            cerr << "could not open " << fncs_log_filename << endl;
        }
        Output2Tee::Stream1() = stdout;
        Output2Tee::Stream2() = pFile;
    }
    else if (log_stdout) {
        Output2Tee::Stream1() = stdout;
        Output2Tee::Stream2() = NULL;
    }
    else if (log_file) {
        FILE *pFile = fopen(fncs_log_filename, "a");
        if (!pFile) {
            cerr << "could not open " << fncs_log_filename << endl;
        }
        Output2Tee::Stream1() = pFile;
        Output2Tee::Stream2() = NULL;
    }

    FNCSLog::ReportingLevel() = FNCSLog::FromString(fncs_log_level);
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
        /* create an empty ZPL file in case all info was provided on
         * command line */
        config = zconfig_new("root", NULL);
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

    /* create a zchunk for parsing */
    zchunk = zchunk_new(configuration.c_str(), configuration.size());

    /* open and parse fncs configuration */
    config = zconfig_chunk_load(zchunk);
    if (!config) {
        cerr << "could not load configuration chunk" << endl;
        cerr << "-- configuration was as follows --" << endl;
        cerr << configuration << endl;
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

    fncs::start_logging();

    /* whether die() should exit() */
    fatal = getenv("FNCS_FATAL");
    if (!fatal) {
        fatal =  "yes";
    }
    if (fatal[0] == 'N'
            || fatal[0] == 'n'
            || fatal[0] == 'F'
            || fatal[0] == 'f') {
        LINFO << "fncs::die() will not call exit()";
        die_is_fatal = false;
    }
    else {
        LINFO << "fncs::die() will call exit(EXIT_FAILURE)";
        die_is_fatal = true;
    }

    /* broker location from env var is tried first */
    broker_endpoint = getenv("FNCS_BROKER");
    if (!broker_endpoint) {
        /* read broker location from config */
        broker_endpoint = zconfig_resolve(config, "/broker", NULL);
        if (!broker_endpoint) {
            LINFO << "fncs config does not contain 'broker'";
            LINFO << "broker default is tcp://localhost:5570";
            broker_endpoint = "tcp://localhost:5570";
        }
    }
    else {
        LINFO << "FNCS_BROKER env var sets the broker endpoint location";
        /* don't need to send broker address to broker */
        /*zconfig_put(config, "/broker", broker_endpoint);*/
    }
    LDEBUG << "broker = " << broker_endpoint;

    /* time_delta from env var is tried first */
    time_delta_string = getenv("FNCS_TIME_DELTA");
    if (!time_delta_string) {
        /* read time delta from config */
        time_delta_string = zconfig_resolve(config, "/time_delta", NULL);
        if (!time_delta_string) {
            LWARNING << "fncs config does not contain 'time_delta'";
            LWARNING << "time_delta default is 1s";
            time_delta_string = "1s";
        }
    }
    else {
        LINFO << "FNCS_TIME_DELTA env var sets the time_delta_string";
        /* put what we got from env var so it sends on to broker */
        zconfig_put(config, "/time_delta", time_delta_string);
    }
    LDEBUG << "time_delta_string = " << time_delta_string;
    time_delta = parse_time(time_delta_string);
    LDEBUG << "time_delta = " << time_delta;
    time_delta_multiplier = time_unit_to_multiplier(time_delta_string);
    LDEBUG << "time_delta_multiplier = " << time_delta_multiplier;

    /* parse subscriptions */
    config_values = zconfig_locate(config, "/values");
    if (config_values) {
        vector<fncs::Subscription> subs =
            fncs::parse_values(config_values);
        for (size_t i=0; i<subs.size(); ++i) {
            subs_string.insert(make_pair(subs[i].topic, subs[i]));
            LDEBUG2 << "initializing cache for '" << subs[i].key << "'='"
                << subs[i].def << "'";
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
            LDEBUG2 << "'values' appears in config but no subscriptions";
        }
    }
    else {
        LDEBUG2 << "no subscriptions";
    }

    /* create zmq context and client socket */
    client = zsock_new(ZMQ_DEALER);
    if (!client) {
        LERROR << "socket creation failed";
        die();
        return;
    }
    if (!(zsock_resolve(client) != client)) {
        LERROR << "socket failed to resolve";
        die();
        return;
    }

    /* reset the signal handler so it chains */
    signal_handler_reset();

    /* set client identity */
    rc = zmq_setsockopt(zsock_resolve(client), ZMQ_IDENTITY, name, strlen(name));
    if (rc) {
        LERROR << "socket identity failed";
        die();
        return;
    }
    /* finally connect to broker */
    rc = zsock_attach(client, broker_endpoint, false);
    if (rc) {
        LERROR << "socket connection to broker failed";
        die();
        return;
    }

    /* construct HELLO message; entire config goes with it */
    msg = zmsg_new();
    if (!msg) {
        LERROR << "could not construct HELLO message";
        die();
        return;
    }
    rc = zmsg_addstr(msg, HELLO);
    if (rc) {
        LERROR << "failed to append HELLO to message";
        die();
        return;
    }
    zchunk = zconfig_chunk_save(config);
    if (!zchunk) {
        LERROR << "failed to save config for HELLO message";
        die();
        return;
    }
    rc = zmsg_addmem(msg, zchunk_data(zchunk), zchunk_size(zchunk));
    if (rc) {
        LERROR << "failed to add config to HELLO message";
        die();
        return;
    }
    zchunk_destroy(&zchunk);
    LDEBUG2 << "sending HELLO";
    rc = zmsg_send(&msg, client);
    if (rc) {
        LERROR << "failed to send HELLO message";
        die();
        return;
    }

    /* receive ack */
    msg = zmsg_recv(client);
    LDEBUG4 << "called zmsg_recv";
    if (!msg) {
        LERROR << "null message received";
        die();
        return;
    }
    /* first frame is type identifier */
    zframe_t *frame = zmsg_first(msg);
    if (!zframe_streq(frame, ACK)) {
        LERROR << "ACK expected, got " << frame;
        die();
        return;
    }
    LDEBUG2 << "received ACK";
    /* next frame is connetion order ID */
    frame = zmsg_next(msg);
    if (!frame) {
        LERROR << "ACK message missing order ID";
        die();
        return;
    }
    simulation_id = atoi(fncs::to_string(frame).c_str());
    LDEBUG2 << "connection order ID is " << simulation_id;

    /* next frame is n_sims */
    frame = zmsg_next(msg);
    if (!frame) {
        LERROR << "ACK message missing n_sims";
        die();
        return;
    }
    n_sims = atoi(fncs::to_string(frame).c_str());
    LDEBUG2 << "n_sims is " << n_sims;

    zmsg_destroy(&msg);

    is_initialized_ = true;
}


bool fncs::is_initialized()
{
    return is_initialized_;
}


fncs::time fncs::time_request(fncs::time next)
{
    LDEBUG4 << "fncs::time_request(fncs::time)";

    if (!is_initialized_) {
        LWARNING << "fncs is not initialized";
        return next;
    }

    fncs::time granted;

    /* send TIME_REQUEST */
    LDEBUG2 << "sending TIME_REQUEST of " << next << " in sim units";
    next *= time_delta_multiplier;

    if (next % time_delta != 0) {
        LERROR << "time request "
            << next
            << " is not a multiple of time delta ("
            << time_delta
            << ")!";
        die();
        return next;
    }

    LDEBUG1 << "sending TIME_REQUEST of " << next << " nanoseconds";
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

        LDEBUG4 << "entering blocking poll";
        rc = zmq_poll(items, 1, -1);
        if (rc == -1) {
            LERROR << "client polling error: " << strerror(errno);
            die(); /* interrupted */
            return next;
        }

        if (items[0].revents & ZMQ_POLLIN) {
            zmsg_t *msg = NULL;
            zframe_t *frame = NULL;
            string message_type;

            LDEBUG4 << "incoming message";
            msg = zmsg_recv(client);
            if (!msg) {
                LERROR << "null message received";
                die();
                return next;
            }

            /* first frame is message type identifier */
            frame = zmsg_first(msg);
            if (!frame) {
                LERROR << "message missing type identifier";
                die();
                return next;
            }
            message_type = fncs::to_string(frame);

            /* dispatcher */
            if (fncs::TIME_REQUEST == message_type) {
                LDEBUG4 << "TIME_REQUEST received";

                /* next frame is time */
                frame = zmsg_next(msg);
                if (!frame) {
                    LERROR << "message missing time";
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

                LDEBUG4 << "PUBLISH received";

                /* next frame is topic */
                frame = zmsg_next(msg);
                if (!frame) {
                    LERROR << "message missing topic";
                    die();
                    return next;
                }
                topic = fncs::to_string(frame);

                /* next frame is value payload */
                frame = zmsg_next(msg);
                if (!frame) {
                    LERROR << "message missing value";
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
                        LDEBUG4 << "updated cache_list "
                            << "key='" << subscription.key << "' "
                            << "topic='" << topic << "' "
                            << "value='" << value << "' "
                            << "count=" << cache_list[subscription.key].size();
                    } else {
                        cache[subscription.key] = value;
                        LDEBUG4 << "updated cache "
                            << "key='" << subscription.key << "' "
                            << "topic='" << topic << "' "
                            << "value='" << value << "' ";
                    }
                }
                else {
                    LDEBUG4 << "dropping PUBLISH message topic='"
                        << topic << "'";
                }
            }
            else {
                LERROR << "unrecognized message type";
                die();
                return next;
            }

            zmsg_destroy(&msg);
        }
    }

    LDEBUG1 << "granted " << granted << " nanoseonds";
    /* convert nanoseonds to sim's time unit */
    granted = convert_broker_to_sim_time(granted);
    LDEBUG2 << "granted " << granted << " in sim units";
    return granted;
}


void fncs::publish(const string &key, const string &value)
{
    LDEBUG4 << "fncs::publish(string,string)";

    if (!is_initialized_) {
        LWARNING << "fncs is not initialized";
        return;
    }

    string new_key = simulation_name + '/' + key;
    zstr_sendm(client, fncs::PUBLISH);
    zstr_sendm(client, new_key.c_str());
    zstr_send(client, value.c_str());
    LDEBUG4 << "sent PUBLISH '" << new_key << "'='" << value << "'";
}


void fncs::publish_anon(const string &key, const string &value)
{
    LDEBUG4 << "fncs::publish_anon(string,string)";

    if (!is_initialized_) {
        LWARNING << "fncs is not initialized";
        return;
    }

    zstr_sendm(client, fncs::PUBLISH);
    zstr_sendm(client, key.c_str());
    zstr_send(client, value.c_str());
    LDEBUG4 << "sent PUBLISH anon '" << key << "'='" << value << "'";
}


void fncs::route(
        const string &from,
        const string &to,
        const string &key,
        const string &value)
{
    LDEBUG4 << "fncs::route(string,string,string,string)";

    if (!is_initialized_) {
        LWARNING << "fncs is not initialized";
        return;
    }

    string new_key = simulation_name + '/' + from + '@' + to + '/' + key;
    zstr_sendm(client, fncs::PUBLISH);
    zstr_sendm(client, new_key.c_str());
    zstr_send(client, value.c_str());
    LDEBUG4 << "sent PUBLISH '" << new_key << "'='" << value << "'";
}


void fncs::die()
{
    LDEBUG4 << "fncs::die()";

    if (!is_initialized_) {
        LWARNING << "fncs is not initialized";
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
	bool recBye = false;
    LDEBUG4 << "fncs::finalize()";

    if (!is_initialized_) {
        LWARNING << "fncs is not initialized";
        return;
    }

    zmsg_t *msg = NULL;
    zframe_t *frame = NULL;

    zstr_send(client, fncs::BYE);

    /* receive BYE and perhaps other message types */
    zmq_pollitem_t items[] = { { zsock_resolve(client), 0, ZMQ_POLLIN, 0 } };
    while(!recBye){
		/* receive BYE back */
    	int rc = 0;

		LDEBUG4 << "entering blocking poll";
		rc = zmq_poll(items, 1, -1);
		if (rc == -1) {
			LERROR << "client polling error: " << strerror(errno);
			die(); /* interrupted */
			return;
		}
        if (items[0].revents & ZMQ_POLLIN) {
            zmsg_t *msg = NULL;
            zframe_t *frame = NULL;
            string message_type;

            LDEBUG4 << "incoming message";
            msg = zmsg_recv(client);
            if (!msg) {
                LERROR << "null message received";
                die();
                return;
            }

            /* first frame is message type identifier */
            frame = zmsg_first(msg);
            if (!frame) {
                LERROR << "message missing type identifier";
                die();
                return;
            }
            message_type = fncs::to_string(frame);

            if (fncs::TIME_REQUEST == message_type) {
                LERROR << "TIME_REQUEST received. Calling die.";
                die();
                return;
            }
            else if (fncs::PUBLISH == message_type) {
                LDEBUG2 << "PUBLISH received and ignored.";
            }
            else if(fncs::DIE == message_type){
                LERROR << "DIE received.";
                die();
                return;
            }
            else if(fncs::BYE == message_type){
            	LDEBUG4 << "BYE received.";
            	recBye = true;
            }
            else{
            	LERROR << "Unknown message type received! Sending DIE.";
            	die();
            	return;
            }

            zmsg_destroy(&msg);
        }
    }

    zsock_destroy(&client);

    return;
}


void fncs::update_time_delta(fncs::time delta)
{
    LDEBUG4 << "fncs::update_time_delta(fncs::time)";

    if (!is_initialized_) {
        LWARNING << "fncs is not initialized";
        return;
    }

    /* send TIME_DELTA */
    LDEBUG4 << "sending TIME_DELTA of " << delta << " in sim units";
    delta *= time_delta_multiplier;
    LDEBUG4 << "sending TIME_DELTA of " << delta << " nanoseconds";
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
    LDEBUG4 << "fncs::time_unit_to_multiplier(string)";

    fncs::time retval; 
    fncs::time ignore; 
    string unit;
    istringstream iss(value);

    iss >> ignore;
    if (!iss) {
        LERROR << "could not parse time value";
        die();
        return retval;
    }
    iss >> unit;
    if (!iss) {
        LERROR << "could not parse time unit";
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
        LERROR << "unrecognized time unit '" << unit << "'";
        die();
        return retval;
    }

    return retval;
}


fncs::time fncs::parse_time(const string &value)
{
    LDEBUG4 << "fncs::parse_time(string)";

    fncs::time retval; 
    string unit;
    istringstream iss(value);

    iss >> retval;
    if (!iss) {
        LERROR << "could not parse time value";
        die();
        return retval;
    }

    retval *= fncs::time_unit_to_multiplier(value);

    return retval;
}


fncs::Subscription fncs::parse_value(zconfig_t *config)
{
    LDEBUG4 << "fncs::parse_value(zconfig_t*)";

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
    LDEBUG4 << "parsing value with key '" << sub.key << "'";

    /* check for topic attached to short key */
    value = zconfig_value(config);
    if (!value || 0 == strlen(value)) {
        LDEBUG4 << "key did not have topic attached";
        /* check for a 'topic' subheading */
        value = zconfig_resolve(config, "topic", NULL);
    }
    if (!value || 0 == strlen(value)) {
        LDEBUG4 << "key did not have 'topic' subheading";
        /* default is to use short key as subscription */
        sub.topic = sub.key;
    }
    else {
        sub.topic = value;
    }
    LDEBUG4 << "parsing key '" << sub.key << "' topic '" << sub.topic << "'";

    value = zconfig_resolve(config, "default", NULL);
    if (!value) {
        LDEBUG4 << "parsing value '" << sub.key << "', missing 'default'";
    }
    sub.def = value? value : "";

    value = zconfig_resolve(config, "type", NULL);
    if (!value) {
        LDEBUG4 << "parsing value '" << sub.key << "', missing 'type'";
    }
    sub.type = value? value : "double";

    value = zconfig_resolve(config, "list", NULL);
    if (!value) {
        LDEBUG4 << "parsing value '" << sub.key << "', missing 'list'";
    }
    sub.list = value? value : "false";

    return sub;
}


vector<fncs::Subscription> fncs::parse_values(zconfig_t *config)
{
    LDEBUG4 << "fncs::parse_values(zconfig_t*)";

    vector<fncs::Subscription> subs;
    string name;
    zconfig_t *child = NULL;

    name = zconfig_name(config);
    if (name != "values") {
        LERROR << "error parsing 'values', wrong config object '" << name << "'";
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
    LDEBUG4 << "fncs::get_events() [" << events.size() << "]";

    if (!is_initialized_) {
        LWARNING << "fncs is not initialized";
        return vector<string>();
    }

    return events;
}


string fncs::get_value(const string &key)
{
    LDEBUG4 << "fncs::get_value(" << key << ")";

    if (!is_initialized_) {
        LWARNING << "fncs is not initialized";
        return "";
    }

    if (0 == cache.count(key)) {
        LERROR << "key '" << key << "' not found in cache";
        die();
        return "";
    }

    return cache[key];
}


vector<string> fncs::get_values(const string &key)
{
    LDEBUG4 << "fncs::get_values(" << key << ")";

    if (!is_initialized_) {
        LWARNING << "fncs is not initialized";
        return vector<string>();
    }

    vector<string> values;

    if (0 == cache_list.count(key)) {
        LERROR << "key '" << key << "' not found in cache list";
        die();
        return values;
    }

    values = cache_list[key];
    LDEBUG4 << "key '" << key << "' has " << values.size() << " values";
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

fncs::time fncs::timer_ft()
{
#ifdef __MACH__
    /* OS X does not have clock_gettime, use clock_get_time */
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    return mts.tv_sec*1000000000UL + mts.tv_nsec;
#elif defined(__FreeBSD__)
    struct timespec ts;
    /* Works on FreeBSD */
    long retval = clock_gettime(CLOCK_MONOTONIC, &ts);
    assert(0 == retval);
    return ts.tv_sec*1000000000UL + ts.tv_nsec;
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
    return 1000000000UL*fncs::time(double(count.QuadPart - start.QuadPart)/freq.QuadPart);
#else
    struct timespec ts;
    /* Works on Linux */
    long retval = clock_gettime(CLOCK_REALTIME, &ts);
    assert(0 == retval);
    return ts.tv_sec*1000000000UL + ts.tv_nsec;
#endif
}

double fncs::timer()
{
    return double(timer_ft())/1000000000.0;
}

