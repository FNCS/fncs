import random
import string

import fncs

name = "testpy"

config = """name = %s
time_delta = 1s""" % name

# generate some time steps
time_steps = sorted(random.sample([i for i in range(100)], 10))

print(config)
fncs.initialize(config.encode('utf-8'))
my_key = "some_key"

for time in time_steps:
    current_time = fncs.time_request(time)
    fncs.publish(my_key, time * 2)
fncs.finalize()
