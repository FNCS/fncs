#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

###############################################################################
# Init autoconf
###############################################################################
AC_PREREQ([2.69])
AC_INIT([Framework for Network Co-Simulation 2],
        [0.1],
        [jeff.daily@pnnl.gov],
        [fncs])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/fncs.hpp])

###############################################################################
# Init automake
###############################################################################
AM_INIT_AUTOMAKE([foreign subdir-objects -Wall -Werror])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

###############################################################################
# Misc. information and package setup.
###############################################################################
FNCS_TOP_BUILDDIR="`pwd`"
cd "$srcdir"
FNCS_TOP_SRCDIR="`pwd`"
cd "$FNCS_TOP_BUILDDIR"
AS_IF([test "$FNCS_TOP_BUILDDIR" != "$FNCS_TOP_SRCDIR"],
      [AC_MSG_NOTICE([Detected VPATH build])])

AC_LANG([C++])

###############################################################################
# C++ compiler
###############################################################################
AC_MSG_NOTICE
AC_MSG_NOTICE([C++ compiler])
AC_MSG_NOTICE

# Checks for programs.
AC_PROG_CXX

# Checks for libraries.
FNCS_CHECK_PACKAGE([zmq], [zmq.h], [zmq], [zmq_ctx_new])
fncs_save_CPPFLAGS="$CPPFLAGS"
fncs_save_LDFLAGS="$LDFLAGS"
fncs_save_LIBS="$LIBS"
CPPFLAGS="$CPPFLAGS $ZMQ_CPPFLAGS"
LDFLAGS="$LDFLAGS $ZMQ_LDFLAGS"
LIBS="$LIBS $ZMQ_LIBS"
FNCS_CHECK_PACKAGE([czmq], [czmq.h], [czmq], [zctx_new])
CPPFLAGS="$fncs_save_CPPFLAGS"
LDFLAGS="$fncs_save_LDFLAGS"
LIBS="$fncs_save_LIBS"

# Checks for header files.
FNCS_CHECK_HEADERS([cstdint])
FNCS_CHECK_HEADERS([stdint.h])
FNCS_CHECK_HEADERS([sys/time.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_CHECK_SIZEOF([int8_t])
AC_CHECK_SIZEOF([int16_t])
AC_CHECK_SIZEOF([int32_t])
AC_CHECK_SIZEOF([int64_t])
AC_CHECK_SIZEOF([uint8_t])
AC_CHECK_SIZEOF([uint16_t])
AC_CHECK_SIZEOF([uint32_t])
AC_CHECK_SIZEOF([uint64_t])
AC_CHECK_SIZEOF([short])
AC_CHECK_SIZEOF([char])
AC_CHECK_SIZEOF([int])
AC_CHECK_SIZEOF([long])
AC_CHECK_SIZEOF([long long])
AC_CHECK_SIZEOF([unsigned char])
AC_CHECK_SIZEOF([unsigned short])
AC_CHECK_SIZEOF([unsigned int])
AC_CHECK_SIZEOF([unsigned long])
AC_CHECK_SIZEOF([unsigned long long])
AC_CHECK_SIZEOF([void*])
FNCS_CXX_NULLPTR

# Checks for library functions.
FNCS_CHECK_FUNCS([gettimeofday])

###############################################################################
# C compiler
###############################################################################
AC_MSG_NOTICE
AC_MSG_NOTICE([C compiler])
AC_MSG_NOTICE
AC_LANG_PUSH([C])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_LANG_POP([C])

###############################################################################
# Libtool setup -- no compiler/linker tests after this
###############################################################################
AC_MSG_NOTICE
AC_MSG_NOTICE([Libtool setup])
AC_MSG_NOTICE

m4_ifdef([AM_PROG_AR],[AM_PROG_AR])

LT_INIT

###############################################################################
# The End
###############################################################################
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

# Report on what we found.
AC_MSG_NOTICE([])
AC_MSG_NOTICE([**************************************************************])
AC_MSG_NOTICE([ $PACKAGE_NAME configured as follows:])
AC_MSG_NOTICE([**************************************************************])
AC_MSG_NOTICE([])
AC_MSG_NOTICE([          CXX=$CXX])
AC_MSG_NOTICE([     CXXFLAGS=$CXXFLAGS])
AC_MSG_NOTICE([          CPP=$CPP])
AC_MSG_NOTICE([     CPPFLAGS=$CPPFLAGS])
AC_MSG_NOTICE([      LDFLAGS=$LDFLAGS])
AC_MSG_NOTICE([         LIBS=$LIBS])
AC_MSG_NOTICE([      MPIEXEC=$MPIEXEC])
AC_MSG_NOTICE([])

