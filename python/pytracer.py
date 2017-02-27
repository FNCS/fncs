import string;
import sys;
import fncs;

time_stop = int(sys.argv[1])
time_granted = 0
op = open (sys.argv[2], "w")

# requires the yaml file
fncs.initialize()

print("# time key value", file=op)

while time_granted < time_stop:
	time_granted = fncs.time_request(time_stop)
	events = fncs.get_events()
	for key in events:
		print (time_granted, key.decode(), fncs.get_value(key).decode(), file=op)

fncs.finalize()
op.close()


