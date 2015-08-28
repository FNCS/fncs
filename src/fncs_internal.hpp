#ifndef _FNCS_INTERNAL_H_
#define _FNCS_INTERNAL_H_
 
#include <cctype>
#include <ostream>
#include <string>
#include <vector>

#include "czmq.h"

#include "echo.hpp"
#include "fncs.hpp"

using ::std::ostream;
using ::std::string;
using ::std::toupper;
using ::std::vector;


FNCS_EXPORT ostream& operator << (ostream& os, zframe_t *self);


namespace fncs {

    class FNCS_EXPORT Subscription {
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

    const char * const HELLO = "hello";
    const char * const ACK = "ack";
    const char * const TIME_REQUEST = "time_request";
    const char * const PUBLISH = "publish";
    const char * const DIE = "die";
    const char * const BYE = "bye";
    const char * const TIME_DELTA = "time_delta";

    /** Connects to broker and parses the given config object. */
    FNCS_EXPORT void initialize(zconfig_t *zconfig);

    /** Starts the FNCS logger. */
    FNCS_EXPORT void start_logging(Echo &echo);

    /** Converts given time string, e.g., '1ms', into a fncs time value.
     * Ignores the value; only converts the unit into a multiplier. */
    FNCS_EXPORT fncs::time time_unit_to_multiplier(const string &value);

    /** Converts given time string, e.g., 1s, into a fncs time value. */
    FNCS_EXPORT fncs::time parse_time(const string &value);

    /** Converts given time value, assumed in ns, to sim's unit. */
    FNCS_EXPORT fncs::time convert_broker_to_sim_time(fncs::time value);

    /** Converts given 'value' zconfig into a fncs Subscription value. */
    FNCS_EXPORT fncs::Subscription parse_value(zconfig_t *config);

    /** Converts all 'values' zconfig items into fncs Subscription values. */
    FNCS_EXPORT vector<fncs::Subscription> parse_values(zconfig_t *config);

    /** Converts given 'match' zconfig into a fncs Subscription value. */
    FNCS_EXPORT fncs::Subscription parse_match(zconfig_t *config);

    /** Converts all 'matches' zconfig items into fncs Subscription matches. */
    FNCS_EXPORT vector<fncs::Subscription> parse_matches(zconfig_t *config);

    /** Converts given czmq frame into a string. */
    FNCS_EXPORT string to_string(zframe_t *frame);

    /** Publish value anonymously using the given key. */
    FNCS_EXPORT void publish_anon(const string &key, const string &value);

    /** Current time in seconds with nanosecond precision. */
    FNCS_EXPORT double timer();
}

#endif /* _FNCS_INTERNAL_H_ */
