/**
 * TODO.
 */
#ifndef _FNCS_H_
#define _FNCS_H_
 
#include <string>

using ::std::string;

namespace fncs {

    typedef unsigned long time;

    /** Connect to broker and parse config file. */
    void initialize();

    /** Connect to broker and parse inline configuration. */
    void initialize(const string &configuration);

    /** Request the next time step to process. */
    time time_request(time next);

    /** Publish value using the given key. */
    void publish(const string &key, const string &value);

    /** Publish value using the given key, adding from:to into the key. */
    void route(const string &from, const string &to, const string &key, const string &value);

    /** Tell broker of a fatal client error. */
    void die();

    /** Close the connection to the broker. */
    void finalize();

}

#endif /* _FNCS_H_ */

