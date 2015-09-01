#!/bin/sh

# locate correct mex program
if test "x$MEX" = x
then
    MEX=`which mex`
fi

if $MEX --version > /dev/null 2>&1
then
    echo "$MEX is the pdfTeX tool, not the MATLAB MEX compiler"
    echo "please update your PATH to locate MATLAB's MEX compiler"
    exit 1
fi

if test "x$FNCS_PREFIX" != x
then
    CPPFLAGS="$CPPFLAGS -I$FNCS_PREFIX/include"
    LDFLAGS="$LDFLAGS -L$FNCS_PREFIX/lib"
    LIBS="$LIBS -lfncs"
    echo "FNCS_PREFIX set to $FNCS_PREFIX"
fi
echo "CPPFLAGS=$CPPFLAGS"
echo " LDFLAGS=$LDFLAGS"
echo "    LIBS=$LIBS"

for cpp in fncs_*.cpp
do
    echo "MEX $cpp"
    $MEX $CPPFLAGS $LDFLAGS $LIBS $cpp || exit 1
done
