import argparse
import json
import string
import sys

import fncs
import yaml

class PyTracer(object):
	def __init__(self,config_file,run_time,output_file):
		self.config_dict = None
		self.run_time = run_time
		self.output_file = output_file
		self.output_data = {}
		try:
			with open(config_file, "r") as config_file_stream:
				self.config_dict = yaml.load(config_file_stream)
		except yaml.YAMLError as e:
			print("There was and error loading the yaml configuration. Error:\n" + e)
			self.config_dict = None
		finally:
			if self.config_dict != None:
				config_string = yaml.dump(self.config_dict, default_flow_style=False)
				print(config_string)
				fncs.initialize(config_string)
				if fncs.is_initialized():
					print("successful registration with the fncs_broker.\n")
				else:
					print("registration with the fncs_broker failed.\n")
	
	
	def runFederate(self):
		time_granted = 0
		is_simulation_complete = False
		value = 0
		while time_granted <= self.run_time:
			if fncs.is_initialized():
				events = fncs.get_events()
				print("events = %s" % events)
				self.output_data[time_granted] = {}
				for key in events:
					if fncs.is_initialized():
						self.output_data[time_granted][key] = fncs.get_value(key)
				value += 1 
				if time_granted < self.run_time and fncs.is_initialized:
					print("pytracer - calling fncs.time_request(%s)" % self.run_time)
					time_granted = fncs.time_request(self.run_time)
					if time_granted > self.run_time:
						fncs.die()
				elif time_granted >= self.run_time:
					break
				
					
			if not fncs.is_initialized():
				break
			
		if fncs.is_initialized():
			fncs.finalize()
		op = open(self.output_file, "w")
		topic_start = len(str(max(self.output_data.keys()))) + 4
		if topic_start < 11:
			topic_start = 11
		topic_len_max = 0
		for d in self.output_data.keys():
			for key in self.output_data[d].keys():
				if len(key) > topic_len_max:
					topic_len_max = len(key)
		value_start = topic_start + topic_len_max + 4
		if value_start < 20:
			value_start = 20
		print([topic_start, topic_len_max, value_start])
		op.write("Time(s)")
		num_spaces = topic_start - 7
		for sp in range(num_spaces):
			op.write(" ")
		op.write("Topic")
		num_spaces  = value_start - 5 - topic_start
		for sp in range(num_spaces):
			op.write(" ")
		op.write("Value\n")
		for t in sorted(self.output_data.keys()):
			for topic in sorted(self.output_data[t].keys()):
				len_written = 0
				op.write("%s" % str(t))
				num_spaces = topic_start - len(str(t))
				for sp in range(num_spaces):
					op.write(" ")
				op.write("%s" % topic)
				num_spaces = value_start - len(topic) - topic_start
				for sp in range(num_spaces):
					op.write(" ")
				op.write("%s\n" % self.output_data[t][topic])
		op.close()
		
		
def main():
	parser = argparse.ArgumentParser()
	parser.add_argument("configuration_file", type=str, help="The full path for the FNCS configuration file.")
	parser.add_argument("run_time", type=int, help="How long the tracer will run in seconds.")
	parser.add_argument("output_file", type=str, help="The name of the tracer output file.")
	args = parser.parse_args()
	tracer = PyTracer(args.configuration_file, args.run_time, args.output_file)
	tracer.runFederate()
	
	
		
if __name__ == '__main__':
	main()
	
	