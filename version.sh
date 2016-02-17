#!/bin/sh
#
# This script extracts the FNCS version from src/fncs.hpp, which is the master
# location for this information.
#
if [ ! -f src/fncs.hpp ]; then
    echo "version.sh: error: src/fncs.hpp does not exist" 1>&2
    exit 1
fi
MAJOR=`egrep '^#define +FNCS_VERSION_MAJOR +[0-9]+$' src/fncs.hpp`
MINOR=`egrep '^#define +FNCS_VERSION_MINOR +[0-9]+$' src/fncs.hpp`
PATCH=`egrep '^#define +FNCS_VERSION_PATCH +[0-9]+$' src/fncs.hpp`
if [ -z "$MAJOR" -o -z "$MINOR" -o -z "$PATCH" ]; then
    echo "version.sh: error: could not extract version from src/fncs.hpp" 1>&2
    exit 1
fi
MAJOR=`echo $MAJOR | awk '{ print $3 }'`
MINOR=`echo $MINOR | awk '{ print $3 }'`
PATCH=`echo $PATCH | awk '{ print $3 }'`
echo $MAJOR.$MINOR.$PATCH | tr -d '\n'

