/* autoconf header */
#include "config.h"

/* C++ standard headers */
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <map>
#include <set>
#include <string>
#include <sstream>
#include <vector>

/* 3rd party headers */
#include "czmq.h"
#include "easylogging++.h"
_INITIALIZE_EASYLOGGINGPP

/* fncs headers */
#include "fncs.hpp"
#include "fncs_internal.hpp"

using namespace ::easyloggingpp;
using namespace ::std;


class SimulatorState {
    public:
        SimulatorState()
            : name("")
            , time_requested(0)
            , time_delta(0)
            , time_last_processed(0)
            , processing(true)
            , messages_pending(false)
        {}

        string name;
        fncs::time time_requested;
        fncs::time time_delta;
        fncs::time time_last_processed;
        bool processing;
        bool messages_pending;
        vector<zrex_t*> subscriptions;
};

typedef map<string,size_t> SimIndex;
typedef vector<SimulatorState> SimVec;


static inline void broker_die(const SimVec &simulators, zsock_t *server) {
    /* repeat the fatal die to all connected sims */
    for (size_t i=0; i<simulators.size(); ++i) {
        zstr_sendm(server, simulators[i].name.c_str());
        zstr_send(server, fncs::DIE);
    }
    zsock_destroy(&server);
    exit(EXIT_FAILURE);
}


int main(int argc, char **argv)
{
    /* declare all variables */
    unsigned int n_sims = 0;    /* how many sims will connect */
    set<string> byes;           /* which sims have disconnected */
    int n_processing = 0;       /* how many sims are processing a time step */
    const char *endpoint = NULL;/* broker location */
    SimVec simulators;          /* vector of connected simulator state */
    SimIndex name_to_index;     /* quickly lookup sim state index */
    fncs::time time_granted = 0;/* global clock */
    zsock_t *server = NULL;     /* the broker socket */

    fncs::start_logging();

    /* how many simulators are connecting? */
    if (argc > 2) {
        LFATAL << "too many command line args";
        exit(1);
    }
    else if (argc < 2) {
        LFATAL << "missing command line arg for number of simulators";
        exit(1);
    }
    else {
        int n_sims_signed = 0;
        istringstream iss(argv[1]);
        iss >> n_sims_signed;
        LTRACE << "n_sims_signed = " << n_sims_signed;
        if (n_sims_signed <= 0) {
            LFATAL << "number of simulators arg must be >= 1";
            exit(1);
        }
        n_sims = static_cast<unsigned int>(n_sims_signed);
    }

    /* broker endpoint may come from env var */
    endpoint = getenv("FNCS_BROKER");
    if (!endpoint) {
        endpoint = "tcp://*:5570";
    }

    server = zsock_new_router(endpoint);
    if (!server) {
        LFATAL << "socket creation failed";
    }
    if (!(zsock_resolve(server) != server)) {
        LFATAL << "socket failed to resolve";
    }
    LTRACE << "broker socket bound to " << endpoint;

    /* begin event loop */
    zmq_pollitem_t items[] = { { zsock_resolve(server), 0, ZMQ_POLLIN, 0 } };
    while (true) {
        int rc = 0;
        
        LTRACE << "entering blocking poll";
        rc = zmq_poll(items, 1, -1);
        if (rc == -1) {
            LTRACE << "broker polling error: " << strerror(errno);
            broker_die(simulators, server); /* interrupted */
        }

        if (items[0].revents & ZMQ_POLLIN) {
            zmsg_t *msg = NULL;
            zframe_t *frame = NULL;
            string sender;
            string message_type;

            LTRACE << "incoming message";
            msg = zmsg_recv(server);
            if (!msg) {
                LFATAL << "null message received";
                broker_die(simulators, server);
            }

            /* first frame is sender */
            frame = zmsg_first(msg);
            if (!frame) {
                LFATAL << "message missing sender";
                broker_die(simulators, server);
            }
            LTRACE << frame;
            sender = fncs::to_string(frame);

            /* next frame is message type identifier */
            frame = zmsg_next(msg);
            if (!frame) {
                LFATAL << "message missing type identifier";
                broker_die(simulators, server);
            }
            LTRACE << frame;
            message_type = fncs::to_string(frame);
            
            /* dispatcher */
            if (fncs::HELLO == message_type) {
                SimulatorState state;
                zchunk_t *chunk = NULL;
                zconfig_t *config = NULL;
                zconfig_t *config_values = NULL;
                const char * time_delta = NULL;

                LTRACE << "HELLO received";

                /* check for duplicate sims */
                if (name_to_index.count(sender) != 0) {
                    LFATAL << "simulator '" << sender << "' already connected";
                    exit(1);
                }
                LTRACE << "registering client '" << sender << "'";

                /* next frame is config chunk */
                frame = zmsg_next(msg);
                if (!frame) {
                    LFATAL << "HELLO message missing config frame";
                    broker_die(simulators, server);
                }
                LTRACE << frame;

                /* copy config frame into chunk */
                chunk = zchunk_new(zframe_data(frame), zframe_size(frame));
                if (!chunk) {
                    LFATAL << "HELLO message zconfig bad config chunk";
                    broker_die(simulators, server);
                }

                /* parse config chunk */
                config = zconfig_chunk_load(chunk);
                if (!config) {
                    LFATAL << "HELLO message bad config";
                    broker_die(simulators, server);
                }

                /* done with chunk */
                zchunk_destroy(&chunk);

                /* get time delta from config */
                time_delta = zconfig_resolve(config, "/time_delta", NULL);
                if (!time_delta) {
                    LFATAL << sender << " config does not contain 'time_delta'";
                    broker_die(simulators, server);
                }

                /* parse subscriptions */
                vector<zrex_t*> subscriptions;
                config_values = zconfig_locate(config, "/values");
                if (config_values) {
                    vector<fncs::Subscription> subs =
                        fncs::parse_values(config_values);
                    for (size_t i=0; i<subs.size(); ++i) {
                        LTRACE << "compiling re'" << subs[i].topic << "'";
                        subscriptions.push_back(
                                zrex_new(subs[i].topic.c_str()));
                    }
                }
                else {
                    LTRACE << "no subscriptions";
                }
                config_values = zconfig_locate(config, "/matches");
                if (config_values) {
                    vector<fncs::Subscription> subs =
                        fncs::parse_matches(config_values);
                    for (size_t i=0; i<subs.size(); ++i) {
                        LTRACE << "compiling re'" << subs[i].topic << "'";
                        subscriptions.push_back(
                                zrex_new(subs[i].topic.c_str()));
                    }
                }
                else {
                    LTRACE << "no matches";
                }

                /* populate sim state object */
                state.name = sender;
                state.time_delta = fncs::parse_time(time_delta);
                state.time_requested = 0;
                state.time_last_processed = 0;
                state.processing = false;
                state.messages_pending = false;
                state.subscriptions = subscriptions;
                name_to_index[sender] = simulators.size();
                simulators.push_back(state);

                LTRACE << "simulators.size() = " << simulators.size();

                /* if all sims have connected, send the go-ahead */
                if (simulators.size() == n_sims) {
                    /* easier to keep a counter than iterating over states */
                    n_processing = n_sims;
                    /* send ACK to all registered sims */
                    for (size_t i=0; i<n_sims; ++i) {
                        simulators[i].processing = true;
                        zstr_sendm(server, simulators[i].name.c_str());
                        zstr_sendm(server, fncs::ACK);
                        zstr_sendfm(server, "%llu", (unsigned long long)i);
                        zstr_sendf(server, "%llu", (unsigned long long)n_sims);
                        LTRACE << "ACK sent to '" << simulators[i].name;
                    }
                }
            }
            else if (fncs::TIME_REQUEST == message_type) {
                size_t index = 0; /* index of sim state */
                fncs::time time_requested;

                LTRACE << "TIME_REQUEST received";

                /* did we receive message from a connected sim? */
                if (name_to_index.count(sender) == 0) {
                    LFATAL << "simulator '" << sender << "' not connected";
                    broker_die(simulators, server);
                }

                /* index of sim state */
                index = name_to_index[sender];

                /* next frame is time */
                frame = zmsg_next(msg);
                if (!frame) {
                    LFATAL << "TIME_REQUEST message missing time frame";
                    broker_die(simulators, server);
                }
                LTRACE << frame;
                /* convert time string */
                {
                    istringstream iss(fncs::to_string(frame));
                    iss >> time_requested;
                }

                /* update sim state */
                simulators[index].time_requested = time_requested;
                simulators[index].time_last_processed = time_granted;
                simulators[index].processing = false;

                --n_processing;

                /* if all sims are done, determine next time step */
                if (0 == n_processing) {
                    vector< fncs::time> time_actionable(n_sims);
                    for (size_t i=0; i<n_sims; ++i) {
                        if (simulators[i].messages_pending) {
                            time_actionable[i] = 
                                  simulators[i].time_last_processed
                                + simulators[i].time_delta;
                        }
                        else {
                            time_actionable[i] = simulators[i].time_requested;
                        }
                    }
                    time_granted = *min_element(time_actionable.begin(),
                                                time_actionable.end());
                    LTRACE << "time_granted = " << time_granted;
                    for (size_t i=0; i<n_sims; ++i) {
                        if (time_granted == time_actionable[i]) {
                            ++n_processing;
                            simulators[i].processing = true;
                            simulators[i].messages_pending = false;
                            zstr_sendm(server, simulators[i].name.c_str());
                            zstr_sendm(server, fncs::TIME_REQUEST);
                            zstr_sendf(server, "%llu", time_granted);
                        }
                    }
                }
            }
            else if (fncs::PUBLISH == message_type) {
                string topic = "";
                bool found_one = false;

                LTRACE << "PUBLISH received";

                /* did we receive message from a connected sim? */
                if (name_to_index.count(sender) == 0) {
                    LFATAL << "simulator '" << sender << "' not connected";
                    broker_die(simulators, server);
                }

                /* next frame is topic */
                frame = zmsg_next(msg);
                if (!frame) {
                    LFATAL << "PUBLISH message missing topic";
                    broker_die(simulators, server);
                }
                LTRACE << frame;
                topic = fncs::to_string(frame);

                /* send the message to subscribed sims */
                for (size_t i=0; i<n_sims; ++i) {
                    for (size_t j=0; j<simulators[i].subscriptions.size(); ++j) {
                        if (zrex_matches(
                                    simulators[i].subscriptions[j],
                                    topic.c_str())) {
                            zmsg_t *msg_copy = zmsg_dup(msg);
                            if (!msg_copy) {
                                LFATAL << "failed to copy pub message";
                                broker_die(simulators, server);
                            }
                            /* swap out original sender with new destiation */
                            zframe_reset(zmsg_first(msg_copy),
                                    simulators[i].name.c_str(),
                                    simulators[i].name.size());
                            /* send it on */
                            zmsg_send(&msg_copy, server);
                            found_one = true;
                            simulators[i].messages_pending = true;
                            /* even if multiple subscriptions for the
                             * current simulator match this message, we
                             * only want to send it once */
                            break;
                        }
                    }
                }
                if (!found_one) {
                    LWARNING << "dropping PUBLISH message '" << topic << "'";
                }
            }
            else if (fncs::DIE == message_type) {
                LTRACE << "DIE received";

                /* did we receive message from a connected sim? */
                if (name_to_index.count(sender) == 0) {
                    LFATAL << "simulator '" << sender << "' not connected";
                    broker_die(simulators, server);
                }

                broker_die(simulators, server);
            }
            else if (fncs::BYE == message_type) {
                size_t index = 0; /* index of sim state */

                LTRACE << "BYE received";

                /* did we receive message from a connected sim? */
                if (name_to_index.count(sender) == 0) {
                    LFATAL << "simulator '" << sender << "' not connected";
                    broker_die(simulators, server);
                }

                /* index of sim state */
                index = name_to_index[sender];

                /* soft error if muliple byes received */
                if (byes.count(sender)) {
                    LERROR << "duplicate BYE from '" << sender << "'";
                }

                /* add sender to list of leaving sims */
                byes.insert(sender);
                simulators[index].processing = false;

                /* if all byes received, then exit */
                if (byes.size() == n_sims) {
                    /* let all sims know that globally we are finished */
                    for (size_t i=0; i<n_sims; ++i) {
                        zstr_sendm(server, simulators[i].name.c_str());
                        zstr_send(server, fncs::BYE);
                        LTRACE << "BYE sent to '" << simulators[i].name;
                    }
                    /* need to delete msg since we are breaking from loop */
                    zmsg_destroy(&msg);
                    break;
                }
            }
            else {
                LFATAL << "received unknown message type '"
                    << message_type << "'";
                broker_die(simulators, server);
            }

            zmsg_destroy(&msg);
        }
    }

    zsock_destroy(&server);

    return 0;
}

