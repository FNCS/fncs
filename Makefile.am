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

AM_CPPFLAGS += $(ZMQ_CPPFLAGS)

AM_LDFLAGS += $(ZMQ_LDFLAGS)

LDADD += libfncs.la
LDADD += $(ZMQ_LIBS)

include_HEADERS += include/fncs.hpp

AM_CPPFLAGS += -I$(top_srcdir)/include

lib_LTLIBRARIES += libfncs.la
libfncs_la_SOURCES =
libfncs_la_SOURCES += src/fncs.cpp

check_PROGRAMS += tests/test
tests_test_SOURCES = tests/test.cpp

