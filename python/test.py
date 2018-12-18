import random
import string

import fncs

name = "randome_name_" + "".join( [random.choice(string.digits) for i in xrange(8)] )

config = """name = %s
time_delta = 1s""" % name

# generate some time steps
time_steps = sorted(random.sample([i for i in xrange(100)], 10))
print time_steps

fncs.initialize(config)

for time in time_steps:
    current_time = fncs.time_request(time)
    print "current time is", current_time
    fncs.publish("some_key", "some_value")
fncs.finalize()
