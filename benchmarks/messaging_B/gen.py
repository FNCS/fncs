#!/usr/bin/env python

import sys

if len(sys.argv) != 3:
    print "usage: gen.py <N sims> <N messages each>"
    sys.exit(1)

lower = "abcdefghijklmnopqrstuvwxyz"
KEY = []

for i in range(len(lower)):
    for j in range(len(lower)):
        KEY.append(lower[i]+lower[j])

print """name = d0
time_delta = 1s
values"""
for i in range(int(sys.argv[1])):
    sim = "d%s" % i
    for j in range(int(sys.argv[2])):
        msg = KEY[j]
        print "    %s/%s" % (sim,msg)
