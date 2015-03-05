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

AM_CPPFLAGS += -I$(top_srcdir)/contrib
AM_CPPFLAGS += -I$(top_srcdir)/src
AM_CPPFLAGS += $(CZMQ_CPPFLAGS)
AM_CPPFLAGS += $(ZMQ_CPPFLAGS)

AM_LDFLAGS += $(CZMQ_LDFLAGS)
AM_LDFLAGS += $(ZMQ_LDFLAGS)

LDADD += libfncs.la
LDADD += $(CZMQ_LIBS)
LDADD += $(ZMQ_LIBS)

include_HEADERS += src/fncs.hpp

lib_LTLIBRARIES += libfncs.la
libfncs_la_SOURCES =
libfncs_la_SOURCES += src/fncs.cpp
libfncs_la_SOURCES += src/fncs_internal.hpp
libfncs_la_LIBADD =
libfncs_la_LIBADD += $(CZMQ_LIBS)
libfncs_la_LIBADD += $(ZMQ_LIBS)

check_PROGRAMS += tests/test
tests_test_SOURCES = tests/test.cpp

bin_PROGRAMS += fncs_broker
fncs_broker_SOURCES = src/broker.cpp
