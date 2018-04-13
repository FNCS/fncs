/* autoconf header */
#include "config.h"

/* C++  standard headers */
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <set>
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

/* 3rd party contrib */
#include "yaml-cpp/yaml.h"
#include "json-cpp/json/json.h"

/* fncs headers */
#include "log.hpp"
#include "fncs.hpp"
#include "fncs_internal.hpp"

using namespace ::std;

static bool is_initialized_ = false;
static bool die_is_fatal = false;
static bool aggregate_sub = false;
static bool aggregate_pub = false;
static bool time_fixed = false;
static string simulation_name = "";
static string agent_subtopic = "Output";
static int simulation_id = 0;
static int n_sims = 0;
static fncs::time time_delta_multiplier = 0;
static fncs::time time_delta = 0;
static fncs::time time_peer = 0;
static fncs::time time_current = 0;
static fncs::time time_window = 0;
static zsock_t *client = NULL;
static map<string,string> pub_cache;
static map<string,string> cache;
static vector<string> events;
static set<string> keys; /* keys that other sims subscribed to */
static vector<string> mykeys; /* keys from the fncs config file */
static long poll_timeout = -1;

static const string default_broker = "tcp://localhost:5570";
static const string default_time_delta = "1s";
static const string default_fatal = "yes";
static const string default_aggregate_sub = "no";
static const string default_aggregate_pub = "no";
static const string default_time_fixed = "no";
static const long   default_poll_timeout = -1;

typedef map<string,vector<string> > clist_t;
static clist_t cache_list;

typedef map<string,fncs::Subscription> sub_string_t;
static sub_string_t subs_string;

#ifdef INSTRUMENTATION
/* INSTRUMENTATION part I starts */
/* NEW VARIABLES FOR INSTRUMENTATION */
static fncs::time start_clock = 0;
static fncs::time start_time = 0;
/* float req_clock = 0;
float grant_clock = 0; */
unsigned int num_grant = 0;
unsigned int num_req = 0;
static fncs::time req_time = 0;
static fncs::time grant_time = 0;

static vector<fncs::time> vec_wait_time;
static vector<fncs::time> vec_time_next;
static vector<fncs::time> vec_time_granted;
/* INSTRUMENTATION part I ends */
#endif


#if defined(_WIN32)
static BOOL WINAPI
s_handler_fn_shim (DWORD ctrltype)
{
    if (ctrltype == CTRL_C_EVENT) {
        zctx_interrupted = 1;
        zsys_interrupted = 1;
    }
    return FALSE;
}
static void signal_handler_reset()
{
    zsys_handler_set(NULL);
    SetConsoleCtrlHandler(s_handler_fn_shim, TRUE);
}
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

#if 0
static inline string nodetype(const YAML::Node &node) {
    if (node.Type() == YAML::NodeType::Scalar) { return "SCALAR"; } 
    else if (node.Type() == YAML::NodeType::Sequence) { return "SEQUENCE"; } 
    else if (node.Type() == YAML::NodeType::Map) { return "MAP"; } 
    else { return "ERROR"; }
}
#endif

static inline bool EndsWith(const string& a, const string& b) {
    if (b.size() > a.size()) return false;
    return std::equal(a.begin() + a.size() - b.size(), a.end(), b.begin());
}


void fncs::start_logging()
{
    const char *fncs_log_filename = NULL;
    const char *fncs_log_stdout = NULL;
    const char *fncs_log_file = NULL;
    const char *fncs_log_level = NULL;
    string simlog = simulation_name + ".log";
    bool log_file = false;
    bool log_stdout = true;

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
        fncs_log_stdout = "yes";
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
        log_stdout = false;
    }

    if (fncs_log_file[0] == 'Y'
            || fncs_log_file[0] == 'y'
            || fncs_log_file[0] == 'T'
            || fncs_log_file[0] == 't') {
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


void fncs::replicate_logging(TLogLevel &level, FILE *& one, FILE *& two)
{
    level = FNCSLog::ReportingLevel();
    one = Output2Tee::Stream1();
    two = Output2Tee::Stream2();
}


/* This version of initialize() checks for a config filename in the
 * environment and then defaults to a known name. */
void fncs::initialize()
{
    const char *fncs_config_file = NULL;
    zconfig_t *zconfig = NULL;
    fncs::Config config;

    /* name for fncs config file from environment */
    fncs_config_file = getenv("FNCS_CONFIG_FILE");
    if (!fncs_config_file) {
        fncs_config_file = "fncs.zpl";
    }

    if (EndsWith(fncs_config_file, "zpl")) {
        zconfig = zconfig_load(fncs_config_file);
        if (zconfig) {
            config = parse_config(zconfig);
            zconfig_destroy(&zconfig);
        }
        else {
            cerr << "could not open " << fncs_config_file << endl;
        }
    }
    else if (EndsWith(fncs_config_file, "yaml")) {
        try {
            ifstream fin(fncs_config_file);
            YAML::Parser parser(fin);
            YAML::Node doc;
            parser.GetNextDocument(doc);
            config = parse_config(doc);
        } catch (YAML::ParserException &) {
            cerr << "could not open " << fncs_config_file << endl;
        }
    }
    else {
        cerr << "fncs config file must end in *.zpl or *.yaml" << endl;
    }

    initialize(config);
}


/* This version of initialize() reads configuration parameters directly
 * from the given string. */
void fncs::initialize(const string &configuration)
{
    zchunk_t *zchunk = NULL;
    zconfig_t *zconfig = NULL;
    fncs::Config config;

    config = parse_config(configuration);

    initialize(config);
}

/* This function allows applications to use json structure as the initialize()
 * configuration. */
void fncs::agentRegister()
{
    const char *fncs_config_file = NULL;
    fncs::Config config;
    /* name for fncs config file from environment */
    fncs_config_file = getenv("FNCS_CONFIG_FILE");
    if (!fncs_config_file) {
        fncs_config_file = "fncs.json";
    }
    if (EndsWith(fncs_config_file, "json")) {
        ifstream fin(fncs_config_file);
        Json::Value json_config;
        try {
            fin >> json_config;
        } catch (Json::Exception &) {
            cerr << "could not open " << fncs_config_file << endl;
        }
        config = parse_config(json_config);
    }
    initialize(config);
}


void fncs::agentRegister(const string &configuration)
{
    fncs::Config config;
    Json::Value json_config;
    Json::Reader json_reader;
    try {
        json_reader.parse(configuration, json_config);
    } catch (Json::Exception &) {
        cerr << "could not recognize the configuration as a json string. " << configuration << endl;
    }
    config = parse_config(json_config);

    initialize(config);
}


void fncs::initialize(Config config)
{
    const char *env_fatal = NULL;
    const char *env_name = NULL;
    const char *env_broker = NULL;
    const char *env_time_delta = NULL;
    const char *env_poll_timeout = NULL;
    int rc;
    zmsg_t *msg = NULL;
    zchunk_t *zchunk = NULL;
    zconfig_t *config_values = NULL;

    /* name from env var overrides config file */
    env_name = getenv("FNCS_NAME");
    if (env_name) {
        config.name = env_name;
    }
    else if (config.name.empty()) {
        cerr << "FNCS_NAME env var not set and" << endl;
        cerr << "fncs config does not contain 'name'" << endl;
        die();
        return;
    }
    simulation_name = config.name;

    fncs::start_logging();

    /* whether die() should exit() */
    env_fatal = getenv("FNCS_FATAL");
    if (env_fatal) {
        config.fatal = env_fatal;
        LINFO << "fatal set by FNCS_FATAL env var";
    }
    else if (config.fatal.empty()) {
        LINFO << "FNCS_FATAL env var not set and fncs config does not contain 'fatal'";
        LINFO << "defaulting to " << default_fatal;
        config.fatal = default_fatal;
    }
    {
        char fc = config.fatal[0];
        if (fc == 'N' || fc == 'n' || fc == 'F' || fc == 'f') {
            die_is_fatal = false;
            LINFO << "fncs::die() will not call exit()";
        }
        else {
            die_is_fatal = true;
            LINFO << "fncs::die() will call exit(EXIT_FAILURE)";
        }
    }

    /* whether we want to receive aggregate publications */
    if (config.aggregate_sub.empty()) {
        LINFO << "fncs config does not contain 'aggregate_sub'";
        LINFO << "defaulting to " << default_aggregate_sub;
        config.aggregate_sub = default_aggregate_sub;
    }
    {
        char fc = config.aggregate_sub[0];
        if (fc == 'N' || fc == 'n' || fc == 'F' || fc == 'f') {
            LINFO << "one-shot subscription messages";
            aggregate_sub = false;
        }
        else {
            LINFO << "aggregate subscription messages";
            aggregate_sub = true;
        }
    }

    /* whether we want to produce aggregate publications */
    if (config.aggregate_pub.empty()) {
        LINFO << "fncs config does not contain 'aggregate_pub'";
        LINFO << "defaulting to " << default_aggregate_pub;
        config.aggregate_pub = default_aggregate_pub;
    }
    {
        char fc = config.aggregate_pub[0];
        if (fc == 'N' || fc == 'n' || fc == 'F' || fc == 'f') {
            LINFO << "one-shot publication messages";
            aggregate_pub = false;
        }
        else {
            LINFO << "aggregate publication messages";
            aggregate_pub = true;
        }
    }

    /* whether we want time requests to be absolutely fixed */
    if (config.time_fixed.empty()) {
        LINFO << "fncs config does not contain 'time_fixed'";
        LINFO << "defaulting to " << default_time_fixed;
        config.time_fixed = default_time_fixed;
    }
    {
        char fc = config.time_fixed[0];
        if (fc == 'N' || fc == 'n' || fc == 'F' || fc == 'f') {
            LINFO << "time requests are the normal default";
            time_fixed = false;
        }
        else {
            LINFO << "fixed/absolute time requests (non-interruptible)";
            time_fixed = true;
        }
    }

    /* broker location from env var overrides config file */
    env_broker = getenv("FNCS_BROKER");
    if (env_broker) {
        LINFO << "FNCS_BROKER env var sets the broker endpoint location";
        config.broker = env_broker;
    } else if (config.broker.empty()) {
        LINFO << "FNCS_BROKER env var not set and fncs config does not contain 'broker'";
        LINFO << "defaulting to " << default_broker;
        config.broker = default_broker;
    }
    LDEBUG << "broker = " << config.broker;

    /* time_delta from env var is tried first */
    env_time_delta = getenv("FNCS_TIME_DELTA");
    if (env_time_delta) {
        LINFO << "FNCS_TIME_DELTA env var sets the time_delta";
        config.time_delta = env_time_delta;
    } else if (config.time_delta.empty()) {
        LINFO << "FNCS_TIME_DELTA env var not set and fncs config does not contain 'time_delta'";
        LINFO << "defaulting to " << default_time_delta;
        config.time_delta = default_time_delta;
    }
    LDEBUG << "time_delta string = " << config.time_delta;
    time_delta = parse_time(config.time_delta);
    LDEBUG << "time_delta = " << time_delta;
    time_delta_multiplier = time_unit_to_multiplier(config.time_delta);
    LDEBUG << "time_delta_multiplier = " << time_delta_multiplier;

    env_poll_timeout = getenv("FNCS_POLL_TIMEOUT");
    if (env_poll_timeout) {
        LINFO << "FNCS_POLL_TIMEOUT env var sets the poll_timeout";
        istringstream iss(env_poll_timeout);
        iss >> poll_timeout;
        if (!iss) {
            LERROR << "could not parse poll_timeout";
            die();
            return;
        }
    } else {
        poll_timeout = default_poll_timeout;
    }
    LDEBUG << "poll_timeout = " << poll_timeout;
    if (poll_timeout < -1) {
        LERROR << "poll timeout must be >= -1";
        die();
        return;
    }

    /* parse subscriptions */
    {
        vector<Subscription> subs = config.values;
        for (size_t i=0; i<subs.size(); ++i) {
            subs_string.insert(make_pair(subs[i].topic, subs[i]));
            mykeys.push_back(subs[i].key);
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
            LDEBUG2 << "config did not contain any subscriptions";
        }
    }

    zsys_set_sndhwm(0);
    zsys_set_rcvhwm(0);
    
    
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


    zsock_set_linger(client, -1);
    
    
    //cout << "Message size: " << zsock_maxmsgsize(client);

    /* reset the signal handler so it chains */
    signal_handler_reset();

    /* set client identity */
    rc = zmq_setsockopt(zsock_resolve(client), ZMQ_IDENTITY, config.name.c_str(), config.name.size());
    if (rc) {
        LERROR << "socket identity failed";
        die();
        return;
    }
    /* finally connect to broker */
    rc = zsock_attach(client, config.broker.c_str(), false);
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
    LDEBUG2 << "-- sending configuration as follows --" << endl << config.to_string();
    rc = zmsg_addstr(msg, config.to_string().c_str());
    if (rc) {
        LERROR << "failed to save config for HELLO message";
        die();
        return;
    }
    string version_string;
    {
        ostringstream oss;
        oss << FNCS_VERSION_MAJOR << "." << FNCS_VERSION_MINOR << "." << FNCS_VERSION_PATCH;
        version_string = oss.str();
    }
    rc = zmsg_addstr(msg, version_string.c_str());
    if (rc) {
        LERROR << "failed to append version to message";
        die();
        return;
    }
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

    /* next frame is number of subscription keys */
    frame = zmsg_next(msg);
    if (!frame) {
        LERROR << "ACK message missing n_keys";
        die();
        return;
    }
    int n_keys = atoi(fncs::to_string(frame).c_str());
    LDEBUG2 << "n_keys is " << n_keys;

    /* next frames are the keys */
    for (int i=0; i<n_keys; ++i) {
        frame = zmsg_next(msg);
        if (!frame) {
            LERROR << "ACK message missing key " << i;
            die();
            return;
        }
        string key = fncs::to_string(frame);
        keys.insert(key);
        LDEBUG2 << "key is " << key;
    }

    /* next frame is peer time */
    frame = zmsg_next(msg);
    if (!frame) {
        LERROR << "ACK message missing peer time";
        die();
        return;
    }
    fncs::time time_peer_long = strtoull(fncs::to_string(frame).c_str(), NULL, 0);
    LDEBUG2 << "time_peer_long is " << time_peer_long << " parsed from " << fncs::to_string(frame);
    time_peer = time_peer_long;

    /* next frame is FNCS library version */
    frame = zmsg_next(msg);
    if (!frame) {
        LWARNING << "HELLO message from broker missing FNCS library version";
    }
    else {
        string version_string = fncs::to_string(frame);
        int result[3];
        std::istringstream parser(version_string);
        parser >> result[0];
        for(int idx = 1; idx < 3; idx++) {
            parser.get(); //Skip period
            parser >> result[idx];
        }
        if (FNCS_VERSION_MAJOR != result[0]
                || FNCS_VERSION_MINOR != result[1]
                || FNCS_VERSION_PATCH != result[2]) {
            LWARNING << "FNCS library version mismatch, client="
                << FNCS_VERSION_MAJOR << "."
                << FNCS_VERSION_MINOR << "."
                << FNCS_VERSION_PATCH
                << " broker="
                << version_string;
        }
    }

    /* last frame is second ACK */
    frame = zmsg_next(msg);
    if (!zframe_streq(frame, ACK)) {
        LERROR << "ACK expected, got " << frame;
        die();
        return;
    }
    LDEBUG2 << "received second ACK";
    zmsg_destroy(&msg);

    time_current = 0;
    time_window = 0;
    is_initialized_ = true;

# ifdef INSTRUMENTATION
    /* INSTRUMENTATION PART II STARTS 
    start_clock = clock(); */
    start_time = (timer_ft());
    /* ofstream datafile;
    datafile.open("instrumentation.csv", ios::out | ios::app);
    datafile << simulation_name << "," << start_clock << "," << start_time << "\n";
    INSTRUMENTATION PART II ENDS */
#endif
}


bool fncs::is_initialized()
{
    return is_initialized_;
}


fncs::time fncs::time_request(fncs::time time_next)
{
    LDEBUG4 << "fncs::time_request(fncs::time)";

    if (!is_initialized_) {
        LWARNING << "fncs is not initialized";
        return time_next;
    }

    if (aggregate_pub && !pub_cache.empty()) {
        LDEBUG2 << "sending PUBLISH_AGGREGATE";
        zstr_sendm(client, fncs::PUBLISH_AGGREGATE);
        zstr_sendfm(client, "%llu", (unsigned long long)pub_cache.size());
        for (map<string,string>::const_iterator it=pub_cache.begin();
                it != pub_cache.end(); ++it) {
            zstr_sendm(client, it->first.c_str());
            zstr_sendm(client, it->second.c_str());
        }
        zstr_send(client, "END");
        pub_cache.clear();
    }

    fncs::time time_granted;
    fncs::time time_passed;

    /* send TIME_REQUEST */
    LDEBUG2 << "sending TIME_REQUEST of " << time_next << " in sim units";
    time_next *= time_delta_multiplier;

    if (time_next % time_delta != 0) {
        LERROR << "time request "
            << time_next
            << " ns is not a multiple of time delta ("
            << time_delta
            << " ns)!";
        die();
        return time_next;
    }

    if (time_next < time_current) {
        LERROR << "time request "
            << time_next
            << " ns is smaller than the current time ("
            << time_current
            << " ns)!";
        die();
        return time_next;
    }

    time_passed = time_next - time_current;
    LDEBUG2 << "time advanced " << time_passed << " ns since last request";

    /* sending of the time request implies we are done with the cache
     * list, but the other cache remains as a last value cache */
    /* only clear the vectors associated with cache list keys because
     * the keys should remain valid i.e. empty lists are meaningful */
    events.clear();
    for (clist_t::iterator it=cache_list.begin(); it!=cache_list.end(); ++it) {
        it->second.clear();
    }

    if (time_passed < time_window) {
        time_window -= time_passed;
        LDEBUG1 << "there are " << time_window << " nanoseconds left in the window";
        LDEBUG1 << "time_granted " << time_next << " nanoseonds";
        time_current = time_next;
        /* convert nanoseonds to sim's time unit */
        time_granted = convert_broker_to_sim_time(time_next);
        LDEBUG2 << "time_granted " << time_granted << " in sim units";
        return time_granted;
    }
    else {
        LDEBUG1 << "time_window expired";
        time_window = 0;
    }

    LDEBUG1 << "sending TIME_REQUEST of " << time_next << " nanoseconds";

#ifdef INSTRUMENTATION
    /* INSTRUMENTATION PART III BEGINS
    req_clock = clock()-start_clock; */
    req_time = (timer_ft()); /*-start_time); */
    num_req = num_req +1;

    vec_time_next.push_back(time_next);

    /* ofstream datafile;
    datafile.open("instrumentation.csv", ios::out | ios::app);
    datafile << "Request" << "," << simulation_name << "," << num_req << "," << req_time << "," << time_next << "\n";
    INSTRUMENTATION PART III ENDS */
#endif

    zstr_sendm(client, fncs::TIME_REQUEST);
    zstr_sendfm(client, "%llu", time_next);
    zstr_sendf(client, "%llu", time_current);

    /* receive TIME_REQUEST and perhaps other message types */
    zmq_pollitem_t items[] = { { zsock_resolve(client), 0, ZMQ_POLLIN, 0 } };
    while (true) {
        int rc = 0;
        LDEBUG4 << "entering blocking poll" ;
        fncs::time poll_start = timer();
        fncs::time poll_wait = -1;
        rc = zmq_poll(items, 1, poll_timeout);
        
        // If the poll times out no items will be received.
        if (rc == 0) {
            // Returning zero is used to indicate to the mini_federate a timeout has occurred.
            return 0;
        }
        if (rc == -1) {
            poll_wait = timer() - poll_start;
            LERROR << "client polling error  " << poll_wait << " seconds after entering poll:"  << strerror(errno);
            die(); /* interrupted */
            return time_next;
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
                return time_next;
            }

            /* first frame is message type identifier */
            frame = zmsg_first(msg);
            if (!frame) {
                LERROR << "message missing type identifier";
                die();
                return time_next;
            }
            message_type = fncs::to_string(frame);

            /* dispatcher */
            if (fncs::TIME_REQUEST == message_type) {
                LDEBUG4 << "TIME_REQUEST received";

                /* time_next frame is time */
                frame = zmsg_next(msg);
                if (!frame) {
                    LERROR << "message missing time";
                    die();
                    return time_next;
                }
                /* convert time string to nanoseconds */
                {
                    istringstream iss(fncs::to_string(frame));
                    iss >> time_granted;
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
                    return time_next;
                }
                topic = fncs::to_string(frame);

                /* next frame is value payload */
                frame = zmsg_next(msg);
                if (!frame) {
                    LERROR << "message missing value";
                    die();
                    return time_next;
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
                            << "topic='" << topic << "' ";
                           // << "value='" << value << "' ";
                    }
                }
                else {
                    LDEBUG4 << "dropping PUBLISH message topic='"
                        << topic << "'";
                }
            }
            else if (fncs::PUBLISH_AGGREGATE == message_type) {
                LDEBUG4 << "PUBLISH_AGGREGATE received";
                /* next frame is topic/value count */
                frame = zmsg_next(msg);
                if (!frame) {
                    LERROR << "message missing topic/value count";
                    die();
                    return time_next;
                }
                int n_topics = atoi(fncs::to_string(frame).c_str());
                /* iterate over each topic/value pair */
                for (int i=0; i<n_topics; ++i) {
                    string topic;
                    string value;

                    /* next frame is topic */
                    frame = zmsg_next(msg);
                    if (!frame) {
                        LERROR << "message missing topic";
                        die();
                        return time_next;
                    }
                    topic = fncs::to_string(frame);

                    /* next frame is value */
                    frame = zmsg_next(msg);
                    if (!frame) {
                        LERROR << "message missing value";
                        die();
                        return time_next;
                    }
                    value = fncs::to_string(frame);

                    sub_string_t::const_iterator sub_str_itr;
                    fncs::Subscription subscription;
                    bool found = false;

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
                                << "topic='" << topic << "' ";
                            // << "value='" << value << "' ";
                        }
                    }
                    else {
                        LDEBUG4 << "dropping PUBLISH_AGGREGATE message topic='"
                            << topic << "'";
                    }
                }
            }
            else if  (fncs::DIE == message_type){
                poll_wait = timer() - poll_start;
                LDEBUG4 << "DIE recieved " << poll_wait << " seconds after entering blocking poll";
                die();
                return time_next;
            }  
            else {
                LERROR << "unrecognized message type: " << message_type;
                die();
                return time_next;
            }

            zmsg_destroy(&msg);
        }
    }

    LDEBUG1 << "time_granted " << time_granted << " nanoseonds";
    if (time_fixed && time_granted != time_next) {
        LERROR << "fixed time setting was active, but time granted was not time requested";
        die();
        return time_next;
    }

#ifdef INSTRUMENTATION
    /* INSTRUMENTATION PART IV BEGINS
    grant_clock = clock() - start_clock; */
    grant_time = (timer_ft()); /* - start_time); */
    num_grant = num_grant +1;

    vec_wait_time.push_back(grant_time - req_time);
    vec_time_granted.push_back(time_granted);
    /* datafile << "Grant" << "," << simulation_name << "," << num_grant << "," << grant_time << "," << time_granted << "\n";
    INSTRUMENTATION PART IV ENDS */
#endif

    time_current = time_granted;

    /* the peers this sim interacts with have a larger 'tick' */
    if (time_peer > time_delta) {
        /* If we were granted a time that is evenly divisible by our
         * peer time, we can't create a time window just yet -- if the
         * peer published we wouldn't get the message until after the
         * window expired which would be too late. */
        if (time_current % time_peer != 0) {
            /* how much time is left before reaching the peers' time? */
            time_window = time_peer - (time_current % time_peer);
            LDEBUG1 << "new time_window of " << time_window << " nanoseconds";
        }
    }

    /* convert nanoseonds to sim's time unit */
    time_granted = convert_broker_to_sim_time(time_granted);
    LDEBUG2 << "time_granted " << time_granted << " in sim units";

    return time_granted;
}


void fncs::publish(const string &key, const string &value)
{
    LDEBUG4 << "fncs::publish(string,string)";

    if (!is_initialized_) {
        LWARNING << "fncs is not initialized";
        return;
    }

    if (keys.count(key)) {
        string new_key = simulation_name + '/' + key;
        if (aggregate_pub) {
            pub_cache[new_key] = value;
            LDEBUG4 << "cached PUBLISH '" << new_key << "'='" << value << "'";
        }
        else {
            zstr_sendm(client, fncs::PUBLISH);
            zstr_sendm(client, new_key.c_str());
            zstr_send(client, value.c_str());
            LDEBUG4 << "sent PUBLISH '" << new_key << "'='" << value << "'";
        }
    }
    else {
        LDEBUG4 << "dropped " << key;
    }
}


void fncs::publish_anon(const string &key, const string &value)
{
    LDEBUG4 << "fncs::publish_anon(string,string)";

    if (!is_initialized_) {
        LWARNING << "fncs is not initialized";
        return;
    }

    if (aggregate_pub) {
        pub_cache[key] = value;
        LDEBUG4 << "cached PUBLISH anon '" << key << "'='" << value << "'";
    }
    else {
        zstr_sendm(client, fncs::PUBLISH);
        zstr_sendm(client, key.c_str());
        zstr_send(client, value.c_str());
        LDEBUG4 << "sent PUBLISH anon '" << key << "'='" << value << "'";
    }
}


void fncs::agentPublish(const string &value)
{
    string agent_key = simulation_name + "/TransactiveAgentOutput";
    LDEBUG4 << "fncs::agentPublish(string)";

    if (!is_initialized_) {
        LWARNING << "fncs is not initialized";
        return;
    }

    zstr_sendm(client, fncs::PUBLISH);
    zstr_sendm(client, agent_key.c_str());
    zstr_send(client, value.c_str());
    LDEBUG4 << "sent PUBLISH anon '" << agent_key << "'='" << value << "'";
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

    if (aggregate_pub) {
        pub_cache[new_key] = value;
        LDEBUG4 << "cached PUBLISH '" << new_key << "'='" << value << "'";
    }
    else {
        zstr_sendm(client, fncs::PUBLISH);
        zstr_sendm(client, new_key.c_str());
        zstr_send(client, value.c_str());
        LDEBUG4 << "sent PUBLISH '" << new_key << "'='" << value << "'";
    }
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
        zsys_shutdown(); /* without this, Windows will hang */
    }

    is_initialized_ = false;

    if (die_is_fatal) {
        exit(EXIT_FAILURE);
    }
}


void fncs::finalize()
{
#ifdef INSTRUMENTATION
/* INSTRUMENTATION PART V STARTS */
    ofstream myfile;
    myfile.open("time_request.csv", ios::out | ios::trunc);
    myfile << "Time of initialization" << "," << start_time << endl;

    myfile << "Requested time" << "," << "Granted time" "," <<  "fncs::time_request wait time" << endl;
    int vsize = vec_wait_time.size();
    for(int vn=0; vn<vsize; vn++){
        myfile << vec_time_next[vn] << "," << vec_time_granted[vn] << "," << vec_wait_time[vn] << endl;
    }
#endif
/* INSTRUMENTATION PART V end */


    bool recBye = false;
    LDEBUG4 << "fncs::finalize()";

    if (!is_initialized_) {
        LWARNING << "fncs is not initialized";
        return;
    }

    zmsg_t *msg = NULL;
    zframe_t *frame = NULL;

    zstr_sendm(client, fncs::BYE);
    zstr_sendf(client, "%llu", time_current);

    /* receive BYE and perhaps other message types */
    zmq_pollitem_t items[] = { { zsock_resolve(client), 0, ZMQ_POLLIN, 0 } };
    while(!recBye){
        /* receive BYE back */
        int rc = 0;
        fncs::time poll_start = timer();
        LDEBUG4 << "entering blocking poll";
        rc = zmq_poll(items, 1, -1);
        if (rc == -1) {
            fncs::time poll_wait = timer() - poll_start;
            LERROR << "client polling error " << poll_wait << " seconds after entering blocking poll: " << strerror(errno);
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
    zsys_shutdown(); /* without this, Windows will hang */

    is_initialized_ = false;
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

    /* update our client-side value */
    time_delta = delta;
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

    fncs::time retval = 0;
    fncs::time ignore = 0;
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


fncs::Config fncs::parse_config(const string &configuration)
{
    LDEBUG4 << "fncs::parse_config(string)";

    zchunk_t *zchunk = NULL;
    zconfig_t *zconfig = NULL;
    fncs::Config config;
    bool is_zpl = false;

    /* if we detect '=' in the first line, we assume zpl */
    {
        istringstream is(configuration);
        string line;
        getline(is, line);
        is_zpl = (line.find('=') != string::npos);
    }

    if (is_zpl) {
        /* create a zchunk for parsing */
        zchunk = zchunk_new(configuration.c_str(), configuration.size());
        /* open and parse fncs configuration */
        zconfig = zconfig_chunk_load(zchunk);
        if (!zconfig) {
            cerr << "could not load ZPL configuration string" << endl;
            cerr << "-- configuration was as follows --" << endl;
            cerr << configuration << endl;
        }
        else {
            zchunk_destroy(&zchunk);
            config = parse_config(zconfig);
            zconfig_destroy(&zconfig);
        }
    }
    else {
        /* attempt to load as a YAML document */
        try {
            istringstream sin(configuration);
            YAML::Parser parser(sin);
            YAML::Node doc;
            parser.GetNextDocument(doc);
            config = parse_config(doc);
        } catch (YAML::ParserException &) {
            cerr << "could not load YAML configuration string" << endl;
            cerr << "-- configuration was as follows --" << endl;
            cerr << configuration << endl;
        }
    }

    return config;
}


fncs::Config fncs::parse_config(const YAML::Node &doc)
{
    LDEBUG4 << "fncs::parse_config(YAML::Node)";

    fncs::Config config;

    if (const YAML::Node *node = doc.FindValue("name")) {
        if (node->Type() != YAML::NodeType::Scalar) {
            cerr << "YAML 'name' must be a Scalar" << endl;
        }
        else {
            *node >> config.name;
        }
    }

    if (const YAML::Node *node = doc.FindValue("broker")) {
        if (node->Type() != YAML::NodeType::Scalar) {
            cerr << "YAML 'broker' must be a Scalar" << endl;
        }
        else {
            *node >> config.broker;
        }
    }

    if (const YAML::Node *node = doc.FindValue("time_delta")) {
        if (node->Type() != YAML::NodeType::Scalar) {
            cerr << "YAML 'time_delta' must be a Scalar" << endl;
        }
        else {
            *node >> config.time_delta;
        }
    }

    if (const YAML::Node *node = doc.FindValue("fatal")) {
        if (node->Type() != YAML::NodeType::Scalar) {
            cerr << "YAML 'fatal' must be a Scalar" << endl;
        }
        else {
            *node >> config.fatal;
        }
    }

    if (const YAML::Node *node = doc.FindValue("aggregate_sub")) {
        if (node->Type() != YAML::NodeType::Scalar) {
            cerr << "YAML 'aggregate_sub' must be a Scalar" << endl;
        }
        else {
            *node >> config.aggregate_sub;
        }
    }

    if (const YAML::Node *node = doc.FindValue("aggregate_pub")) {
        if (node->Type() != YAML::NodeType::Scalar) {
            cerr << "YAML 'aggregate_pub' must be a Scalar" << endl;
        }
        else {
            *node >> config.aggregate_pub;
        }
    }

    if (const YAML::Node *node = doc.FindValue("time_fixed")) {
        if (node->Type() != YAML::NodeType::Scalar) {
            cerr << "YAML 'time_fixed' must be a Scalar" << endl;
        }
        else {
            *node >> config.time_fixed;
        }
    }

    /* parse subscriptions */
    if (const YAML::Node *node = doc.FindValue("values")) {
        config.values = parse_values(*node);
    }

    return config;
}


fncs::Config fncs::parse_config(zconfig_t *zconfig)
{
    LDEBUG4 << "fncs::parse_config(zconfig_t*)";

    fncs::Config config;
    zconfig_t *config_values = NULL;

    /* read sim name from zconfig */
    config.name = zconfig_resolve(zconfig, "/name", "");

    /* read broker location from config */
    config.broker = zconfig_resolve(zconfig, "/broker", "");

    /* read time delta from config */
    config.time_delta = zconfig_resolve(zconfig, "/time_delta", "");

    /* read whether die() is fatal from zconfig */
    config.fatal = zconfig_resolve(zconfig, "/fatal", "");

    /* read whether received messages are aggregated from zconfig */
    config.aggregate_sub = zconfig_resolve(zconfig, "/aggregate_sub", "");

    /* read whether published messages are aggregated from zconfig */
    config.aggregate_pub = zconfig_resolve(zconfig, "/aggregate_pub", "");

    config.time_fixed = zconfig_resolve(zconfig, "/time_fixed", "");

    /* parse subscriptions */
    config_values = zconfig_locate(zconfig, "/values");
    if (config_values) {
        config.values = parse_values(config_values);
    }

    return config;
}

fncs::Config fncs::parse_config(const Json::Value &json_config)
{
    fncs::Config config;
    string agentType = "";
    string agentName = "";
    Json::Value publications;
    Json::Value subscriptions;
    Json::Value values;
    vector<string> configurationKeys;
    Json::FastWriter json_writer;
    configurationKeys.push_back("agentType");
    configurationKeys.push_back("agentName");
    configurationKeys.push_back("timeDelta");
    configurationKeys.push_back("broker");
    configurationKeys.push_back("publications");
    configurationKeys.push_back("subscriptions");

    for(vector<string>::iterator i = configurationKeys.begin();
            i != configurationKeys.end(); ++i) {
        if(!json_config.isMember(*i))
            cerr << *i << "is not found in the json configuration" << endl;
    }
    agentType = json_config["agentType"].asString();
    agentName = json_config["agentName"].asString();
    if ((agentType.empty() | agentName.empty())) {
        cerr << "agentType and/or agentName are not defined. Please set both." << endl;
    } else {
        config.name = agentType + "_" + agentName;
    }
    config.time_delta = json_config["time_delta"].asString();
    config.broker = json_config["broker"].asString();
    publications = json_config["publications"];
    subscriptions = json_config["subscriptions"];
    vector<fncs::Subscription> subs;
    for (Json::ValueIterator it = subscriptions.begin(); it != subscriptions.end(); it++) {
        const string subAgentType = it.name();
        for(Json::ValueIterator it1 = subscriptions[it.name()].begin(); it1 != subscriptions[it.name()].end(); it1++) {
            const string subAgentName = it1.name();
            fncs::Subscription newSub;
            Json::Value defaultValue;
            newSub.key = subAgentType + "_" + subAgentName;
            newSub.topic = subAgentType + "_" + subAgentName + "/TransactiveAgentOutput";
            defaultValue[it.name()];
            defaultValue[it.name()][it1.name()] = subscriptions[it.name()][it1.name()];
            const string subDefault = json_writer.write(defaultValue);
            newSub.def = subDefault;
            /* TODO: find a way to specify transactive agent subscriptions as lists */
            newSub.list = "false";
            newSub.type = "JSON";
            subs.push_back(newSub);
        }
    }

    if (json_config.isMember("values")) {
        for (Json::ValueConstIterator itr = json_config["values"].begin(); itr != json_config["values"].end(); itr++) {
            fncs::Subscription valSub;
            valSub.key = itr.name();
            if (json_config["values"][itr.name()].isMember("topic")) {
                valSub.topic = json_config["values"][itr.name()]["topic"].asString();
            } else {
                cerr << "\"topic\" couldn't be found in the json configuration!" << endl;
            }
            if (json_config["values"][itr.name()].isMember("default")) {
                valSub.def = json_config["values"][itr.name()]["default"].asString();
            } else {
                cerr << "\"default\" couldn't be found in the json configuration!" << endl;
            }
            if (json_config["values"][itr.name()].isMember("type")) {
                valSub.type = json_config["values"][itr.name()]["type"].asString();
            } else {
                cerr << "\"type\" couldn't be found in the json configuration!" << endl;
            }
            if (json_config["values"][itr.name()].isMember("list")) {
                valSub.list = json_config["values"][itr.name()]["list"].asString();
            } else {
                cerr << "\"list\" couldn't be found in the json configuration!" << endl;
            }
            subs.push_back(valSub);
        }
    }
    config.values = subs;
    return config;
}


fncs::Subscription fncs::parse_value(const YAML::Node &node)
{
    LDEBUG4 << "fncs::parse_value(YAML::Node)";

    /* a "value" block for a FNCS subscription looks like this
    foo:                    # lookup key, optional topic
        topic:  some_topic  # required iff topic did not appear earlier
        default:  10        # optional; default value
        type:  int          # optional; currently unused; data type
        list:  false        # optional; defaults to "false"
    */

    fncs::Subscription sub;

    if (const YAML::Node *child = node.FindValue("topic")) {
        if (child->Type() != YAML::NodeType::Scalar) {
            cerr << "YAML 'topic' must be a Scalar" << endl;
        }
        else {
            *child >> sub.topic;
        }
    }

    if (const YAML::Node *child = node.FindValue("default")) {
        if (child->Type() != YAML::NodeType::Scalar) {
            cerr << "YAML 'default' must be a Scalar for " << sub.topic << endl;
        }
        else {
            *child >> sub.def;
//            cout << "sub.def " << sub.def << endl;
        }
    }

    if (const YAML::Node *child = node.FindValue("type")) {
        if (child->Type() != YAML::NodeType::Scalar) {
            cerr << "YAML 'type' must be a Scalar for " << sub.topic << endl;
        }
        else {
            *child >> sub.type;
        }
    }

    if (const YAML::Node *child = node.FindValue("list")) {
        if (child->Type() != YAML::NodeType::Scalar) {
            cerr << "YAML 'list' must be a Scalar for " << sub.topic << endl;
        }
        else {
            *child >> sub.list;
        }
    }

    return sub;
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
    sub.type = value? value : "";

    value = zconfig_resolve(config, "list", NULL);
    if (!value) {
        LDEBUG4 << "parsing value '" << sub.key << "', missing 'list'";
    }
    sub.list = value? value : "";

    return sub;
}


vector<fncs::Subscription> fncs::parse_values(const YAML::Node &node)
{
    LDEBUG4 << "fncs::parse_values(YAML::Node)";

    vector<fncs::Subscription> subs;

    if (YAML::NodeType::Sequence == node.Type()) {
        for (YAML::Iterator it=node.begin(); it!=node.end(); ++it) {
            fncs::Subscription sub;
            const YAML::Node &child = *it;
            if (YAML::NodeType::Scalar == child.Type()) {
                child >> sub.key;
                child >> sub.topic;
            }
            else if (YAML::NodeType::Map == child.Type()) {
                for (YAML::Iterator it2=child.begin();
                        it2!=child.end(); ++it2) {
                    if (YAML::NodeType::Scalar == it2.second().Type()) {
                        it2.first() >> sub.key;
                        it2.second() >> sub.topic;
                    }
                    else if (YAML::NodeType::Map == it2.second().Type()) {
                        sub = parse_value(it2.second());
                        it2.first() >> sub.key;
                    }
                }
            }
            subs.push_back(sub);
        }
    }
    else if (YAML::NodeType::Map == node.Type()) {
        for (YAML::Iterator it=node.begin(); it!=node.end(); ++it) {
            fncs::Subscription sub;
            if (YAML::NodeType::Scalar == it.second().Type()) {
                it.first() >> sub.key;
                it.second() >> sub.topic;
            }
            if (YAML::NodeType::Map == it.second().Type()) {
                sub = parse_value(it.second());
                it.first() >> sub.key;
            }
            subs.push_back(sub);
        }
    }

    return subs;
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


string fncs::agentGetEvents()
{
    LDEBUG4 << "fncs::getAgentEvents() [" << events.size() << "]";

    if (!is_initialized_) {
        LWARNING << "fncs is not initialized";
        return string();
    }
    string payload = "";
    string message = "";
    string key = "";
    Json::Value agent_messages;
    Json::Value json_message;
    Json::Reader json_reader;
    Json::StyledWriter json_writer;
    Json::Value default_payload = "";
    vector<string> unique_keys;
    for (vector<string>::iterator keys = events.begin(); keys != events.end(); ++keys) {
        key = *keys;
        bool is_key_unique = true;
        if (unique_keys.size() > 0) {
            for (vector<string>::iterator uk_itr = unique_keys.begin(); uk_itr != unique_keys.end(); ++uk_itr) {
                if (key == *uk_itr) {
                    is_key_unique = false;
                    break;
                }
            }
        }
        if (is_key_unique) {
            message = get_value(key);
            unique_keys.push_back(key);
            json_reader.parse(message, json_message);
            if(json_message.isConvertibleTo(Json::objectValue)){
                bool is_json = false;
                for (Json::ValueIterator itr1 = json_message.begin(); itr1 != json_message.end(); itr1++) {//iterating through agentType
                    is_json = true;
                    if (!agent_messages.isMember(itr1.name())) {
                        agent_messages[itr1.name()];
                    }
                    for (Json::ValueIterator itr2 = json_message[itr1.name()].begin(); itr2 != json_message[itr1.name()].end(); itr2++) {//iterating through agentName
//                        if (agent_messages[itr1.name()].isMember(itr2.name())) {
//                            cerr << "You have recieved more than one message from the same transactive agent during the last time step. This shouldn't have happened." << endl;
//                            die();
//                        }
                        agent_messages[itr1.name()][itr2.name()] = (json_message[itr1.name()][itr2.name()]);
                    }
                }
                if(!is_json) {
                    agent_messages[key] = message;
                }
            } else {
                agent_messages[key] = message;
            }
        }
    }
    if(agent_messages.size() > 0) {
        payload = json_writer.write(agent_messages);
    }
    return payload;
}


string fncs::get_value(const string &key)
{
    LDEBUG4 << "fncs::get_value(" << key << ")";

    if (!is_initialized_) {
        LWARNING << "fncs is not initialized";
        return "";
    }

    if (0 == cache.count(key)) {
        if (0 == cache_list.count(key)) {
            LERROR << "key '" << key << "' not found in either value cache";
        }
        else {
            static set<string> warned;
            if (0 == warned.count(key)) {
                LWARNING << "key '" << key << "' not found in single value cache but was found in multi value cache. Did you mean to use fncs::get_values()? Returning first value in multi value list.";
                warned.insert(key);
            }
            if (0 == cache_list[key].size()) {
                LERROR << "key '" << key << "' had no associated values for this time step.";
                die();
                return "";
            }
            return cache_list[key][0];
        }
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
        LDEBUG4 << "key '" << key << "' not found in multi value cache list";
        if (0 == cache.count(key)) {
            LERROR << "key '" << key << "' not found in either cache list";
            die();
            return values;
        }
        else {
            LDEBUG4 << "key '" << key << "' found only in single value cache list with value '" << cache[key] << "'";
            values.push_back(cache[key]);
        }
    }
    else {
        LDEBUG4 << "key '" << key << "' found in multi value cache list";
        values = cache_list[key];
    }

    LDEBUG4 << "key '" << key << "' has " << values.size() << " values";
    return values;
}


vector<string> fncs::get_keys()
{
    LDEBUG4 << "fncs::get_keys()";

    if (!is_initialized_) {
        LWARNING << "fncs is not initialized";
        return vector<string>();
    }

    return mykeys;
}


string fncs::get_name()
{
    return simulation_name;
}


fncs::time fncs::get_time_delta()
{
    return convert_broker_to_sim_time(time_delta);
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


void fncs::get_version(int *major, int *minor, int *patch)
{
    *major = FNCS_VERSION_MAJOR;
    *minor = FNCS_VERSION_MINOR;
    *patch = FNCS_VERSION_PATCH;
}

