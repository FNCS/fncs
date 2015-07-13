# distutils: language = c++

from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.utility cimport pair

cimport fncshpp as fncs

def initialize(const string &configuration=""):
    if configuration.empty():
        fncs.initialize()
    else:
        fncs.initialize(configuration)

def time_request(fncs.time next):
    return fncs.time_request(next)

def publish(const string &key, const string &value):
    fncs.publish(key, value)

def route(const string &from_, const string &to, const string &key, const string &value):
    fncs.route(from_, to, key, value)

def die():
    fncs.die()

def finalize():
    fncs.finalize()

def update_time_delta(fncs.time delta):
    fncs.update_time_delta(delta)

def get_value(const string &key):
    return fncs.get_value(key)

def get_values(const string &key):
    return fncs.get_values(key)

def get_matches(const string &key):
    return fncs.get_matches(key)

def get_name():
    return fncs.get_name()

def get_id():
    return fncs.get_id()

def get_simulator_count():
    return fncs.get_simulator_count()


