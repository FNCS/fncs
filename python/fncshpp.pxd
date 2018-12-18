# distutils: language = c++

from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.utility cimport pair

cdef extern from "fncs.hpp" namespace "fncs":

    ctypedef unsigned long long time

    void initialize()

    void initialize(const string &configuration)

    bint is_initialized()

    time time_request(time next)

    void publish(const string &key, const string &value)

    void route(const string &from_, const string &to, const string &key, const string &value)

    void die()

    void finalize()

    void update_time_delta(time delta)

    vector[string] get_events()

    string get_value(const string &key)

    vector[string] get_values(const string &key)

    string get_name()

    time get_time_delta()

    int get_id()

    int get_simulator_count()

