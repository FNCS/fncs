import fncs

name = "pycoverage"

config = """name = %s
time_delta = 1s
values
    fcoverage/key
    player/key
    pycoverage/key
    key_anon
        topic = global/key_anon
        list = true
""" % name

# generate some time steps
time_steps = [i*2 for i in xrange(10)]

print "pycoverage test running FNCS version", fncs.get_version()

fncs.initialize(config)
assert fncs.is_initialized()

print "My name is '%s'" % fncs.get_name()
print "I am federate %d out of %d other federates" % (
        fncs.get_id(),
        fncs.get_simulator_count())

for time in time_steps:
    current_time = fncs.time_request(time)
    events = fncs.get_events()
    print "current time is %d, received %d events" % (
            current_time, len(events))
    print "\tevent\tvalue"
    for i,event in enumerate(events):
        if fncs.get_values_size(event) > 1:
            print "\t%s\t%s" % (event, fncs.get_value(event))
        else:
            print "\t%s\t%s" % (event, fncs.get_values(event))
        assert event == fncs.get_event_at(i)
        assert fncs.get_value_at(event, 0) == fncs.get_value(event)
    fncs.publish("key", "value")
    fncs.publish_anon("global/key_anon", name)
    fncs.route("from", "to", "key", "value")

fncs.finalize()
assert not fncs.is_initialized()
