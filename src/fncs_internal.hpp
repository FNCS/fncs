/**
 * TODO.
 */
#ifndef _FNCS_INTERNAL_H_
#define _FNCS_INTERNAL_H_
 
#include <ostream>
#include <string>

#include "czmq.h"

#include "fncs.hpp"

using ::std::ostream;
using ::std::string;


ostream& operator << (ostream& os, zframe_t *self);


namespace fncs {

    class Subscription {
        public:
            Subscription()
                : key("")
                , topic("")
                , def("")
                , type("")
                , list("")
            {}

            string key;
            string topic;
            string def;
            string type;
            string list;
    };

    static const char * HELLO = "hello";
    static const char * ACK = "ack";
    static const char * TIME_REQUEST = "time_request";
    static const char * PUBLISH = "publish";
    static const char * ROUTE = "route";
    static const char * DIE = "die";
    static const char * BYE = "bye";

    /** Connects to broker and parses the given config object. */
    void initialize(zconfig_t *zconfig);

    /** Starts the FNCS logger. */
    void start_logging();

    /** Converts given time string, e.g., '1ms', into a fncs time value.
     * Ignores the value; only converts the unit into a multiplier. */
    fncs::time time_unit_to_multiplier(const string &value);

    /** Converts given time string, e.g., 1s, into a fncs time value. */
    fncs::time time_parse(const string &value);

    /** Converts given 'value' zconfig into a fncs Subscription value. */
    fncs::Subscription subscription_parse(zconfig_t *config);

    /** Converts given czmq frame into a string. */
    string to_string(zframe_t *frame);
}

#endif /* _FNCS_INTERNAL_H_ */
