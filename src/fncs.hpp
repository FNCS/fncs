/**
 * TODO.
 */
#ifndef _FNCS_H_
#define _FNCS_H_
 
#include <string>

using ::std::string;

namespace fncs {

    typedef unsigned long time;

    void initialize();

    void publish(const string &key, const string &value);

    void route(const string &from, const string &to, const string &value);

    void finalize();
}

#endif /* _FNCS_H_ */

