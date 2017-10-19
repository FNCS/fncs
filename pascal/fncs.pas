unit fncs;

{$IFDEF FPC}
{$MODE Delphi}
{$PACKRECORDS C}
{$ENDIF}

{$IFNDEF Windows}
{$linklib fncs}
{$ENDIF}

interface

uses
  unix;

type
  fncs_time = qword;

{$IFDEF Windows}
const
	FNCSLib = 'libfncs';
  // Connect to broker and parse config file.
	procedure fncs_initialize;stdcall;external FNCSLib;
  // Connect to broker and parse inline configuration.
  procedure fncs_initialize_config(configuration:Pchar);stdcall;external FNCSLib;
  // Connect to broker and parse config file for Transactive agents.
  procedure fncs_agentRegister;stdcall;external FNCSLib;
  // Connect to broker and parse inline configuration for transactive agents.
  procedure fncs_agentRegisterConfig(configuration:Pchar);stdcall;external FNCSLib;
  // Check whether simulator is configured and connected to broker.
  function fncs_is_initialized:longint;stdcall;external FNCSLib;
  // Request the next time step to process.
  function fncs_time_request(next:fncs_time):fncs_time;stdcall;external FNCSLib;
  // Publish value using the given key.
  procedure fncs_publish(key:Pchar; value:Pchar);stdcall;external FNCSLib;
  // Publish value anonymously using the given key.
  procedure fncs_publish_anon(key:Pchar; value:Pchar);stdcall;external FNCSLib;
  // Publish function for transactive agents.
  procedure fncs_agentPublish(value:Pchar);stdcall;external FNCSLib;
  // Publish value using the given key, adding from:to into the key.
  procedure fncs_route(from:Pchar; to:Pchar; key:Pchar; value:Pchar);stdcall;external FNCSLib;
  // Tell broker of a fatal client error.
  procedure fncs_die;stdcall;external FNCSLib;
  // Close the connection to the broker.
  procedure fncs_finalize;stdcall;external FNCSLib;
  // Update minimum time delta after connection to broker is made. Assumes time unit is not changing.
  procedure fncs_update_time_delta(delta:fncs_time);stdcall;external FNCSLib;
  // Get the number of keys for all values that were updated during the last time_request.
  function fncs_get_events_size:size_t;stdcall;external FNCSLib;
  // Get the keys for all values that were updated during the last time_request.
  function fncs_get_events:ppchar;stdcall;external FNCSLib;
  // Get one key for the given event index that as updated during the last time_request.
  function fncs_get_event_at(index:size_t):pchar;stdcall;external FNCSLib;
  // Get the agent events for all values that were updated during the last time_request.
  function fncs_agentGetEvents:pchar;stdcall;external FNCSLib;
  // Get a value from the cache with the given key. Will hard fault if key is not found.
  function fncs_get_value(key:Pchar):pchar;stdcall;external FNCSLib;
  // Get the number of values from the cache with the given key.
  function fncs_get_values_size(key:Pchar):size_t;stdcall;external FNCSLib;
  // Get an array of values from the cache with the given key. Will return an array of size 1 if only a single value exists.
  function fncs_get_values(key:Pchar):ppchar;stdcall;external FNCSLib;
  // Get a single value from the array of values for the given key.
  function fncs_get_value_at(key:Pchar; index:size_t):pchar;stdcall;external FNCSLib;
  // Get the number of subscribed keys.
  function fncs_get_keys_size:size_t;stdcall;external FNCSLib;
  // Get the subscribed keys. Will return NULL if fncs_get_keys_size() returns 0.
  function fncs_get_keys:ppchar;stdcall;external FNCSLib;
  // Get the subscribed key at the given index. Will return NULL if fncs_get_keys_size() returns 0.
  function fncs_get_key_at(index:size_t):pchar;stdcall;external FNCSLib;
  // Return the name of the simulator.
  function fncs_get_name:pchar;stdcall;external FNCSLib;
  // Return a unique numeric ID for the simulator.
  function fncs_get_id:longint;stdcall;external FNCSLib;
  // Return the number of simulators connected to the broker.
  function fncs_get_simulator_count:longint;stdcall;external FNCSLib;
  // Run-time API version detection.
  procedure fncs_get_version(major:Plongint; minor:Plongint; patch:Plongint);stdcall;external FNCSLib;
  // Convenience wrapper around libc free.
  procedure _fncs_free(ptr:pointer);stdcall;external FNCSLib;

{$ELSE} // Darwin and Unix

  // Connect to broker and parse config file.
	procedure fncs_initialize;cdecl;external;
  // Connect to broker and parse inline configuration.
  procedure fncs_initialize_config(configuration:Pchar);cdecl;external;
  // Connect to broker and parse config file for Transactive agents.
  procedure fncs_agentRegister;cdecl;external;
  // Connect to broker and parse inline configuration for transactive agents.
  procedure fncs_agentRegisterConfig(configuration:Pchar);cdecl;external;
  // Check whether simulator is configured and connected to broker.
  function fncs_is_initialized:longint;cdecl;external;
  // Request the next time step to process.
  function fncs_time_request(next:fncs_time):fncs_time;cdecl;external;
  // Publish value using the given key.
  procedure fncs_publish(key:Pchar; value:Pchar);cdecl;external;
  // Publish value anonymously using the given key.
  procedure fncs_publish_anon(key:Pchar; value:Pchar);cdecl;external;
  // Publish function for transactive agents.
  procedure fncs_agentPublish(value:Pchar);cdecl;external;
  // Publish value using the given key, adding from:to into the key.
  procedure fncs_route(source:Pchar; target:Pchar; key:Pchar; value:Pchar);cdecl;external;
  // Tell broker of a fatal client error.
  procedure fncs_die;cdecl;external;
  // Close the connection to the broker.
  procedure fncs_finalize;cdecl;external;
  // Update minimum time delta after connection to broker is made. Assumes time unit is not changing.
  procedure fncs_update_time_delta(delta:fncs_time);cdecl;external;
  // Get the number of keys for all values that were updated during the last time_request.
  function fncs_get_events_size:size_t;cdecl;external;
  // Get the keys for all values that were updated during the last time_request.
  function fncs_get_events:ppchar;cdecl;external;
  // Get one key for the given event index that as updated during the last time_request.
  function fncs_get_event_at(index:size_t):pchar;cdecl;external;
  // Get the agent events for all values that were updated during the last time_request.
  function fncs_agentGetEvents:pchar;cdecl;external;
  // Get a value from the cache with the given key. Will hard fault if key is not found.
  function fncs_get_value(key:Pchar):pchar;cdecl;external;
  // Get the number of values from the cache with the given key.
  function fncs_get_values_size(key:Pchar):size_t;cdecl;external;
  // Get an array of values from the cache with the given key. Will return an array of size 1 if only a single value exists.
  function fncs_get_values(key:Pchar):ppchar;cdecl;external;
  // Get a single value from the array of values for the given key.
  function fncs_get_value_at(key:Pchar; index:size_t):pchar;cdecl;external;
  // Get the number of subscribed keys.
  function fncs_get_keys_size:size_t;cdecl;external;
  // Get the subscribed keys. Will return NULL if fncs_get_keys_size() returns 0.
  function fncs_get_keys:ppchar;cdecl;external;
  // Get the subscribed key at the given index. Will return NULL if fncs_get_keys_size() returns 0.
  function fncs_get_key_at(index:size_t):pchar;cdecl;external;
  // Return the name of the simulator.
  function fncs_get_name:pchar;cdecl;external;
  // Return a unique numeric ID for the simulator.
  function fncs_get_id:longint;cdecl;external;
  // Return the number of simulators connected to the broker.
  function fncs_get_simulator_count:longint;cdecl;external;
  // Run-time API version detection.
  procedure fncs_get_version(major:Plongint; minor:Plongint; patch:Plongint);cdecl;external;
  // Convenience wrapper around libc free.
  procedure _fncs_free(ptr:pointer);cdecl;external;

{$ENDIF}

implementation

end.
