/**
 * TODO.
 */
#ifndef _FNCS_INTERNAL_H_
#define _FNCS_INTERNAL_H_
 
#include <cctype>
#include <ostream>
#include <string>
#include <vector>

#include "czmq.h"

#include "fncs.hpp"

using ::std::ostream;
using ::std::string;
using ::std::toupper;
using ::std::vector;


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
                , match(false)
            {}

            string key;
            string topic;
            string def;
            string type;
            string list;
            bool match;

            bool is_list() const {
                return toupper(list[0]) == 'T' || toupper(list[0]) == 'Y';
            }

            bool is_match() const { return match; }
    };

    extern const char * HELLO;
    extern const char * ACK;
    extern const char * TIME_REQUEST;
    extern const char * PUBLISH;
    extern const char * DIE;
    extern const char * BYE;

    /** Connects to broker and parses the given config object. */
    void initialize(zconfig_t *zconfig);

    /** Starts the FNCS logger. */
    void start_logging();

    /** Converts given time string, e.g., '1ms', into a fncs time value.
     * Ignores the value; only converts the unit into a multiplier. */
    fncs::time time_unit_to_multiplier(const string &value);

    /** Converts given time string, e.g., 1s, into a fncs time value. */
    fncs::time parse_time(const string &value);

    /** Converts given time value, assumed in ns, to sim's unit. */
    fncs::time convert_broker_to_sim_time(fncs::time value);

    /** Converts given 'value' zconfig into a fncs Subscription value. */
    fncs::Subscription parse_value(zconfig_t *config);

    /** Converts all 'values' zconfig items into fncs Subscription values. */
    vector<fncs::Subscription> parse_values(zconfig_t *config);

    /** Converts given 'match' zconfig into a fncs Subscription value. */
    fncs::Subscription parse_match(zconfig_t *config);

    /** Converts all 'matches' zconfig items into fncs Subscription matches. */
    vector<fncs::Subscription> parse_matches(zconfig_t *config);

    /** Converts given czmq frame into a string. */
    string to_string(zframe_t *frame);

    /** Publish value anonymously using the given key. */
    void publish_anon(const string &key, const string &value);

}

#endif /* _FNCS_INTERNAL_H_ */
