#!/bin/sh

ECHO_OFF="${ECHO_OFF:-yes}"
#export FNCS_LOG_LEVEL=DEBUG4

if test "x$BLD" = x
then
    echo "must set BLD env var to top level build directory"
    exit 1
fi

if test "x$ECHO_OFF" = xyes
then
    ECHO=:
else
    ECHO=echo
fi

if test "x$FNCS_LOG_LEVEL" = xDEBUG4
then
    T_OUT=t10000.out
else
    T_OUT=/dev/null
fi

export FNCS_BROKER="tcp://172.16.110.43:5570"

$ECHO "FNCS_CONFIG_FILE=t10000.zpl $BLD/benchmarks/bencher 10000 2 16 > $T_OUT &"
FNCS_CONFIG_FILE=t10000.zpl $BLD/benchmarks/bencher 10000 2 16 > $T_OUT &

for i in `seq 5000 9999`
do
    NAME=d$i
    if test "x$FNCS_LOG_LEVEL" = xDEBUG4
    then
        OUT=$NAME.out
    else
        OUT=/dev/null
    fi
    $ECHO "FNCS_CONFIG_FILE=d.zpl FNCS_NAME=$NAME $BLD/benchmarks/bencher 1000 2 16 > $OUT &"
    FNCS_CONFIG_FILE=d.zpl FNCS_NAME=$NAME $BLD/benchmarks/bencher 1000 2 16 > $OUT &
done

echo "launched all simulators, waiting..."
wait
