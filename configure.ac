#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

###############################################################################
# Init autoconf
###############################################################################
AC_PREREQ([2.69])
AC_INIT([Framework for Network Co-Simulation],
        [m4_esyscmd([./version.sh])],
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
AM_MAINTAINER_MODE

###############################################################################
# Misc. information and package setup.
###############################################################################
FNCS_TOP_BUILDDIR="`pwd`"
cd "$srcdir"
FNCS_TOP_SRCDIR="`pwd`"
cd "$FNCS_TOP_BUILDDIR"
AS_IF([test "$FNCS_TOP_BUILDDIR" != "$FNCS_TOP_SRCDIR"],
      [AC_MSG_NOTICE([Detected VPATH build])])

AC_CANONICAL_HOST

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
FNCS_CHECK_PACKAGE([czmq], [czmq.h], [czmq], [zmsg_new])
CPPFLAGS="$fncs_save_CPPFLAGS"
LDFLAGS="$fncs_save_LDFLAGS"
LIBS="$fncs_save_LIBS"

# Set pkgconfigdir
AC_ARG_WITH([pkgconfigdir], AS_HELP_STRING([--with-pkgconfigdir=PATH],
    [Path to the pkgconfig directory [[LIBDIR/pkgconfig]]]),
    [pkgconfigdir="$withval"], [pkgconfigdir='${libdir}/pkgconfig'])
AC_SUBST([pkgconfigdir])

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

# OS-specific tests

libfncs_on_mingw32="no"
libfncs_on_cygwin="no"
libfncs_on_android="no"
libfncs_on_linux="no"
libfncs_on_gnu="no"

case "${host_os}" in
    *linux*)
        libfncs_on_linux="yes"
        case "${host_os}" in
            *android*)
                libfncs_on_android="yes"
            ;;
        esac
    ;;
    *mingw*)
        AC_CHECK_HEADERS(windows.h)
        AC_CHECK_LIB(ws2_32, main, ,
            [AC_MSG_ERROR([cannot link with ws2_32.dll.])])
        AC_CHECK_LIB(rpcrt4, main, ,
            [AC_MSG_ERROR([cannot link with rpcrt4.dll.])])
        AC_CHECK_LIB(iphlpapi, main, ,
            [AC_MSG_ERROR([cannot link with iphlpapi.dll.])])
        libfncs_on_mingw32="yes"
        if test "x$enable_static" = "xyes"; then
            AC_MSG_ERROR([Building static libraries is not supported under MinGW32])
        fi
    ;;
    *cygwin*)
        libfncs_on_cygwin="yes"
    ;;
    gnu*)
        libfncs_on_gnu="yes"
    ;;
esac

AM_CONDITIONAL(ON_MINGW, test "x$libfncs_on_mingw32" = "xyes")
AM_CONDITIONAL(ON_CYGWIN, test "x$libfncs_on_cygwin" = "xyes")
AM_CONDITIONAL(ON_ANDROID, test "x$libfncs_on_android" = "xyes")
AM_CONDITIONAL(ON_LINUX, test "x$libfncs_on_linux" = "xyes")
AM_CONDITIONAL(ON_GNU, test "x$libfncs_on_gnu" = "xyes")


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

# Libtool configuration for different targets
case "${host_os}" in
    *mingw32*|*cygwin*)
        # Disable static build by default
        AC_DISABLE_STATIC
    ;;
    *)
        # Everything else with static enabled
        AC_ENABLE_STATIC
    ;;
esac

# Libtool -version-info (ABI version)
#
# Don't change this unless you know exactly what you're doing and have read and
# understand:
# http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
#
# Changes:
#
# FNCS versions prior to 2.0.1 use 0:0:0 (undefined)
# FNCS versions 2.1.0: 1:0:0 (ABI version 1)
# FNCS versions 2.1.1: 1:0:0 (ABI version 1) - C/C++ library unchanged
# FNCS versions 2.1.2: 1:1:0 (ABI version 1)
# FNCS versions 2.2.0: 2:0:1 (ABI version 2) - C added fncs_is_initialized()
# FNCS versions 2.3.0: 2:1:1 (ABI version 2)
# FNCS versions 2.3.1: 2:1:1 (ABI version 2) - C/C++ library unchanged
#
# libfncs -version-info current:revision:age
LTVER="2:1:1"
AC_SUBST(LTVER)

LT_INIT([win32-dll])

###############################################################################
# The End
###############################################################################
AC_CONFIG_FILES([Makefile src/libfncs.pc])
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

