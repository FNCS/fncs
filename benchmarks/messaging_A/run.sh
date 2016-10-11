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
    BROKER_OUT=fncs_broker.out
    T_OUT=t.out
else
    BROKER_OUT=/dev/null
    T_OUT=/dev/null
fi

$ECHO "$BLD/fncs_broker > $BROKER_OUT 101 &"
$BLD/fncs_broker > $BROKER_OUT 101 &

$ECHO "FNCS_CONFIG_FILE=t.zpl $BLD/benchmarks/bencher 1000 2 16 > $T_OUT &"
FNCS_CONFIG_FILE=t.zpl $BLD/benchmarks/bencher 1000 2 16 > $T_OUT &

for i in `seq 0 99`
do
    NAME=d$i
    if test "x$FNCS_LOG_LEVEL" = xDEBUG4
    then
        OUT=$NAME.out
    else
        OUT=/dev/null
    fi
    $ECHO "FNCS_CONFIG_FILE=d.zpl FNCS_NAME=$NAME $BLD/benchmarks/bencher 100 2 16 > $OUT &"
    FNCS_CONFIG_FILE=d.zpl FNCS_NAME=$NAME $BLD/benchmarks/bencher 100 2 16 > $OUT &
done

echo "launched all simulators, waiting..."
wait
