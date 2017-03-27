ACLOCAL_AMFLAGS = -I m4

include_HEADERS =
lib_LTLIBRARIES =
noinst_LTLIBRARIES =
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
AM_CPPFLAGS += -I$(top_srcdir)/contrib/yaml-cpp/include
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
libfncs_la_LIBADD += libyamlcpp.la
if ON_MINGW
libfncs_la_LDFLAGS = \
    -no-undefined \
    -avoid-version \
    -version-info @LTVER@
else
if ON_CYGWIN
libfncs_la_LDFLAGS = \
    -no-undefined \
    -avoid-version \
    -version-info @LTVER@
else
if ON_ANDROID
libfncs_la_LDFLAGS = \
    -avoid-version \
    -version-info @LTVER@
else
if ON_LINUX
libfncs_la_LDFLAGS = \
    -version-info @LTVER@ \
    -Wl,--version-script=$(srcdir)/src/libfncs.vers
else
libfncs_la_LDFLAGS = \
    -version-info @LTVER@ \
    -Wl
endif
endif
endif
endif

noinst_LTLIBRARIES += libyamlcpp.la
libyamlcpp_la_SOURCES =
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/aliasmanager.cpp
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/binary.cpp
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/collectionstack.h
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/contrib/graphbuilderadapter.cpp
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/contrib/graphbuilderadapter.h
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/contrib/graphbuilder.cpp
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/conversion.cpp
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/directives.cpp
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/directives.h
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/emitfromevents.cpp
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/emitter.cpp
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/emitterstate.cpp
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/emitterstate.h
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/emitterutils.cpp
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/emitterutils.h
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/exp.cpp
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/exp.h
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/indentation.h
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/iterator.cpp
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/iterpriv.h
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/nodebuilder.cpp
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/nodebuilder.h
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/node.cpp
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/nodeownership.cpp
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/nodeownership.h
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/null.cpp
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/ostream.cpp
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/parser.cpp
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/ptr_stack.h
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/ptr_vector.h
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/regex.cpp
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/regex.h
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/regeximpl.h
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/scanner.cpp
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/scanner.h
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/scanscalar.cpp
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/scanscalar.h
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/scantag.cpp
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/scantag.h
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/scantoken.cpp
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/setting.h
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/simplekey.cpp
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/singledocparser.cpp
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/singledocparser.h
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/streamcharsource.h
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/stream.cpp
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/stream.h
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/stringsource.h
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/tag.cpp
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/tag.h
libyamlcpp_la_SOURCES += contrib/yaml-cpp/src/token.h

check_PROGRAMS += contrib/yaml-cpp/test/test
contrib_yaml_cpp_test_test_SOURCES =
contrib_yaml_cpp_test_test_SOURCES += contrib/yaml-cpp/test/emittertests.cpp
contrib_yaml_cpp_test_test_SOURCES += contrib/yaml-cpp/test/emittertests.h
contrib_yaml_cpp_test_test_SOURCES += contrib/yaml-cpp/test/main.cpp
contrib_yaml_cpp_test_test_SOURCES += contrib/yaml-cpp/test/nodetests.h
contrib_yaml_cpp_test_test_SOURCES += contrib/yaml-cpp/test/old-api/parsertests.cpp
contrib_yaml_cpp_test_test_SOURCES += contrib/yaml-cpp/test/old-api/spectests.cpp
contrib_yaml_cpp_test_test_SOURCES += contrib/yaml-cpp/test/parsertests.h
contrib_yaml_cpp_test_test_SOURCES += contrib/yaml-cpp/test/specexamples.h
contrib_yaml_cpp_test_test_SOURCES += contrib/yaml-cpp/test/spectests.cpp
contrib_yaml_cpp_test_test_SOURCES += contrib/yaml-cpp/test/spectests.h
contrib_yaml_cpp_test_test_SOURCES += contrib/yaml-cpp/test/tests.cpp
contrib_yaml_cpp_test_test_SOURCES += contrib/yaml-cpp/test/tests.h

contrib_yaml_cpp_test_test_CPPFLAGS =
contrib_yaml_cpp_test_test_CPPFLAGS += $(AM_CPPFLAGS)
contrib_yaml_cpp_test_test_CPPFLAGS += -I$(top_srcdir)/contrib/yaml-cpp/test

contrib_yaml_cpp_test_test_LDADD = libyamlcpp.la

check_PROGRAMS += tests/config
tests_config_SOURCES = tests/config.cpp

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

bin_PROGRAMS += cffederate
cffederate_SOURCES = src/cffederate.cpp
