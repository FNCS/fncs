#!/bin/bash 
 
 clear 
 
(cd /Users/hard312/testbeds/CCSI_testbed/CCSI_experiments/Experiment_1 && export FNCS_LOG_STDOUT=no && export FNCS_LOG_FILE=no && export FNCS_LOG_LEVEL=ERROR && exec /Users/hard312/testbeds/CCSI_testbed/FNCS_INSTALL/bin/fncs_broker 3 &) 
 sleep 0.01
(cd /Users/hard312/testbeds/CCSI_testbed/CCSI_experiments/Experiment_1/root && export FNCS_LOG_STDOUT=no && export FNCS_LOG_FILE=no && export FNCS_LOG_LEVEL=ERROR && exec /Users/hard312/testbeds/CCSI_testbed/FNCS_INSTALL/bin/mini_federate root 10000ns &) 
sleep 0.01
(cd /Users/hard312/testbeds/CCSI_testbed/CCSI_experiments/Experiment_1/leaf1 && export FNCS_LOG_STDOUT=no && export FNCS_LOG_FILE=no && export FNCS_LOG_LEVEL=ERROR && exec /Users/hard312/testbeds/CCSI_testbed/FNCS_INSTALL/bin/mini_federate leaf 10000ns 23 4 5 &) 
sleep 0.01
(cd /Users/hard312/testbeds/CCSI_testbed/CCSI_experiments/Experiment_1/leaf2 && export FNCS_LOG_STDOUT=no && export FNCS_LOG_FILE=no && export FNCS_LOG_LEVEL=ERROR && exec /Users/hard312/testbeds/CCSI_testbed/FNCS_INSTALL/bin/mini_federate leaf 10000ns 23 4 5 &) 
sleep 0.01

 
 exit 0 
 
