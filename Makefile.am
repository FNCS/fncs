ACLOCAL_AMFLAGS = -I m4

include_HEADERS =
lib_LTLIBRARIES =
bin_PROGRAMS =
check_PROGRAMS =
EXTRA_DIST = README
AM_CXXFLAGS =
AM_CPPFLAGS =
AM_LDFLAGS =
LDADD =

pkgconfig_DATA = src/libfncs.pc

AM_CPPFLAGS += -I$(top_srcdir)/src
AM_CPPFLAGS += -I$(top_srcdir)/contrib
AM_CPPFLAGS += $(CZMQ_CPPFLAGS)
AM_CPPFLAGS += $(ZMQ_CPPFLAGS)

AM_LDFLAGS += $(CZMQ_LDFLAGS)
AM_LDFLAGS += $(ZMQ_LDFLAGS)

LDADD += libfncs.la
LDADD += $(CZMQ_LIBS)
LDADD += $(ZMQ_LIBS)

include_HEADERS += src/fncs.hpp
include_HEADERS += src/fncs.h

lib_LTLIBRARIES += libfncs.la
libfncs_la_SOURCES =
libfncs_la_SOURCES += src/echo.cpp
libfncs_la_SOURCES += src/echo.hpp
libfncs_la_SOURCES += src/fncs.cpp
libfncs_la_SOURCES += src/fncs_capi.cpp
libfncs_la_SOURCES += src/fncs_internal.hpp
libfncs_la_LIBADD =
libfncs_la_LIBADD += $(CZMQ_LIBS)
libfncs_la_LIBADD += $(ZMQ_LIBS)

if ON_MINGW
libfncs_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined -avoid-version
else
libfncs_la_LDFLAGS = $(AM_LDFLAGS)
endif

check_PROGRAMS += tests/test
tests_test_SOURCES = tests/test.cpp

check_PROGRAMS += tests/fncs_netdelay
tests_fncs_netdelay_SOURCES = tests/netdelay.cpp

bin_PROGRAMS += fncs_broker
fncs_broker_SOURCES = src/broker.cpp

bin_PROGRAMS += fncs_tracer
fncs_tracer_SOURCES = src/tracer.cpp

bin_PROGRAMS += fncs_player
fncs_player_SOURCES = src/player.cpp

bin_PROGRAMS += fncs_player_anon
fncs_player_anon_SOURCES = src/player.cpp
fncs_player_anon_CPPFLAGS = $(AM_CPPFLAGS) -DFNCS_ANON
