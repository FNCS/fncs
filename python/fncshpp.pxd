# distutils: language = c++

from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.utility cimport pair

cdef extern from "fncs.hpp" namespace "fncs":

    ctypedef unsigned long long time

    void initialize()

    void initialize(const string &configuration)

    time time_request(time next)

    void publish(const string &key, const string &value)

    void route(const string &from_, const string &to, const string &key, const string &value)

    void die()

    void finalize()

    void update_time_delta(time delta)

    string get_value(const string &key)

    vector[string] get_values(const string &key)

    vector[pair[string,string]] get_matches(const string &key)

    string get_name()

    int get_id()

    int get_simulator_count()

