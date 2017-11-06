import ctypes
import platform

_libname = "libfncs.so"

if platform.system() == 'Darwin':
    _libname = "libfncs.dylib"
elif platform.system() == 'Windows':
    _libname = "libfncs"

_lib = ctypes.CDLL(_libname)

_initialize = _lib.fncs_initialize
_initialize.argtypes = []
_initialize.restype = None

_initialize_config = _lib.fncs_initialize_config
_initialize_config.argtypes = [ctypes.c_char_p]
_initialize_config.restype = None

def initialize(config=None):
    if config:
        _initialize_config(config)
    else:
        _initialize()

_agentRegister = _lib.fncs_agentRegister
_agentRegister.argtypes = []
_agentRegister.restype = None

_agentRegisterConfig = _lib.fncs_agentRegisterConfig
_agentRegisterConfig.argtypes = [ctypes.c_char_p]
_agentRegisterConfig.restype = None

def agentRegister(config=None):
    if config:
        _agentRegisterConfig(config)
    else:
        _agentRegister()

_is_initialized = _lib.fncs_is_initialized
_is_initialized.argtypes = []
_is_initialized.restype = ctypes.c_int

def is_initialized():
    return 1 == _is_initialized()

time_request = _lib.fncs_time_request
time_request.argtypes = [ctypes.c_ulonglong]
time_request.restype = ctypes.c_ulonglong

_publish = _lib.fncs_publish
_publish.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
_publish.restype = None

def publish(key, value):
    _publish(str(key).encode('utf-8'), str(value).encode('utf-8'))

_publish_anon = _lib.fncs_publish_anon
_publish_anon.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
_publish_anon.restype = None

def publish_anon(key, value):
    _publish_anon(str(key).encode('utf-8'), str(value).encode('utf-8'))

_agentPublish = _lib.fncs_agentPublish
_agentPublish.argtypes = [ctypes.c_char_p]
_agentPublish.restype = None

def agentPublish(value):
    _agentPublish(str(value).encode('utf-8'))

route = _lib.fncs_route
route.argtypes = [ctypes.c_char_p,
                  ctypes.c_char_p,
                  ctypes.c_char_p,
                  ctypes.c_char_p]
route.restype = None

die = _lib.fncs_die
die.argtypes = []
die.restype = None

finalize = _lib.fncs_finalize
finalize.argtypes = []
finalize.restype = None

update_time_delta = _lib.fncs_update_time_delta
update_time_delta.argtypes = [ctypes.c_ulonglong]
update_time_delta.restype = None

_free = _lib._fncs_free
_free.argtypes = [ctypes.c_void_p]
_free.restype = None

get_events_size = _lib.fncs_get_events_size
get_events_size.argtypes = []
get_events_size.restype = ctypes.c_size_t

_get_events = _lib.fncs_get_events
_get_events.argtypes = []
_get_events.restype = ctypes.POINTER(ctypes.c_char_p)

def get_events():
    _events = _get_events()
    size = get_events_size()
    events = [_events[i] for i in range(size)]
    for i in range(size):
        _free(_events[i])
    _free(_events)
    return events

_get_event_at = _lib.fncs_get_event_at
_get_event_at.argtypes = [ctypes.c_size_t]
_get_event_at.restype = ctypes.c_char_p

def get_event_at(i):
    _event = _get_event_at(i)
    event = ctypes.string_at(_event)
    _free(_event)
    return event

_agentGetEvents = _lib.fncs_agentGetEvents
_agentGetEvents.argtypes = []
_agentGetEvents.restype = ctypes.c_char_p

def agentGetEvents():
    _event = _agentGetEvents()
    event = ctypes.string_at(_event)
    _free(_event)
    return event

_get_value = _lib.fncs_get_value
_get_value.argtypes = [ctypes.c_char_p]
_get_value.restype = ctypes.c_char_p

def get_value(key):
    _value = _get_value(str(key).encode('utf-8'))
    value = ctypes.string_at(_value)
    _free(_value)
    return value

_get_values_size = _lib.fncs_get_values_size
_get_values_size.argtypes = [ctypes.c_char_p]
_get_values_size.restype = ctypes.c_size_t

def get_values_size(key):
    return _get_values_size(str(key).encode('utf-8'))

_get_values = _lib.fncs_get_values
_get_values.argtypes = [ctypes.c_char_p]
_get_values.restype = ctypes.POINTER(ctypes.c_char_p)

def get_values(key):
    _key = str(key).encode('utf-8')
    _values = _get_values(_key)
    size = get_values_size(_key)
    values = [_values[i] for i in range(size)]
    for i in range(i):
        _free(_values[i])
    _free(_values)
    return values

_get_value_at = _lib.fncs_get_value_at
_get_value_at.argtypes = [ctypes.c_char_p, ctypes.c_size_t]
_get_value_at.restype = ctypes.c_char_p

def get_value_at(key, i):
    _value = _get_value_at(str(key).encode('utf-8'), i)
    value = ctypes.string_at(_value)
    _free(_value)
    return value

get_keys_size = _lib.fncs_get_keys_size
get_keys_size.argtypes = []
get_keys_size.restype = ctypes.c_size_t

_get_keys = _lib.fncs_get_keys
_get_keys.argtypes = []
_get_keys.restype = ctypes.POINTER(ctypes.c_char_p)

def get_keys():
    _keys = _get_keys()
    size = get_keys_size()
    keys = [_keys[i] for i in range(size)]
    for i in range(size):
        _free(_keys[i])
    _free(_keys)
    return keys

_get_key_at = _lib.fncs_get_key_at
_get_key_at.argtypes = [ctypes.c_size_t]
_get_key_at.restype = ctypes.c_char_p

def get_key_at(i):
    _key = _get_key_at(i)
    key = ctypes.string_at(_key)
    _free(_key)
    return key

_get_name = _lib.fncs_get_name
_get_name.argtypes = []
_get_name.restype = ctypes.c_char_p

def get_name():
    _name = _get_name()
    name = ctypes.string_at(_name)
    _free(_name)
    return name

get_id = _lib.fncs_get_id
get_id.argtypes = []
get_id.restype = ctypes.c_int

get_simulator_count = _lib.fncs_get_simulator_count
get_simulator_count.argtypes = []
get_simulator_count.restype = ctypes.c_int

_get_version = _lib.fncs_get_version
_get_version.argtypes = [ctypes.POINTER(ctypes.c_int),
                         ctypes.POINTER(ctypes.c_int),
                         ctypes.POINTER(ctypes.c_int)]
_get_version.restype = None

def get_version():
    major = ctypes.c_int()
    minor = ctypes.c_int()
    patch = ctypes.c_int()
    _get_version(ctypes.byref(major),
                 ctypes.byref(minor),
                 ctypes.byref(patch))
    return (major.value, minor.value, patch.value)
