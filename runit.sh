#!/bin/bash

export PATH=$FNCS_INSTALL/bin:$FNCS_INSTALL/lib:$PATH

eval "$@" | tee $FNCS_LOGGING_FILE
