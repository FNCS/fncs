(exec fncs_broker 2 &> broker.log &)
(exec ./fpc_test &> fpc_test.log &)
(export FNCS_CONFIG_FILE=fpc_tracer.yaml && exec ./fpc_tracer 100 fpc_tracer.out &> fpc_tracer.log &)


