(export FNCS_LOG_STDOUT=yes && export FNCS_LOG_LEVEL=DEBUG4 && exec fncs_broker 2 &> broker.log &)
(export FNCS_LOG_STDOUT=yes && export FNCS_LOG_LEVEL=DEBUG4 && exec python test.py &> test.log &)
(export FNCS_LOG_STDOUT=yes && export FNCS_LOG_LEVEL=DEBUG4 && export FNCS_CONFIG_FILE=tracer.yaml && exec python pytracer.py 100 pytracer.out &> pytracer.log &)
# (export FNCS_LOG_STDOUT=yes && export FNCS_LOG_LEVEL=DEBUG4 && export FNCS_CONFIG_FILE=tracer.yaml && exec fncs_tracer 100s tracer.out &> tracer.log &)

