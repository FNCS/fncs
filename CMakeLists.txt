cmake_minimum_required(VERSION 3.18)
project(fncs)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if(WIN32)
  set(CMAKE_CXX_FLAGS "/EHsc /O2 -D_WINDOWS -D_WIN32 -DLIBFNCS_EXPORTS")
  set(HAVE_WINDOWS_H ON)
  find_library(LIBZMQ_LIBRARY NAMES libzmq-v142-mt-4_3_4)
endif()
if(UNIX)
  set(CMAKE_CXX_FLAGS "-w -O2 -std=c++14 -fPIC")
  set(HAVE_UNISTD_H ON)
  find_library(LIBZMQ_LIBRARY NAMES zmq)
endif()
  if(APPLE)
  set(CMAKE_CXX_FLAGS "-w -O2 -std=c++14")
  set(HAVE_UNISTD_H ON)
  find_library(LIBZMQ_LIBRARY NAMES zmq)
endif()

find_library(CZMQ_LIBRARY czmq)
find_path(CZMQ_INCLUDE_DIR czmq.h)
message("cmzq.h found at " ${CZMQ_INCLUDE_DIR})
message("cmzq library found at " ${CZMQ_LIBRARY})
message("libmzq library found at " ${LIBZMQ_LIBRARY})
include_directories(${CZMQ_INCLUDE_DIR})

configure_file(config.h.in.cmake config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

include_directories(contrib)
include_directories(contrib/yaml-cpp/include)

add_library(jsoncpp STATIC
  contrib/json-cpp/jsoncpp.cpp)

add_library(yamlcpp STATIC
  contrib/yaml-cpp/src/aliasmanager.cpp
  contrib/yaml-cpp/src/binary.cpp
  contrib/yaml-cpp/src/contrib/graphbuilderadapter.cpp
  contrib/yaml-cpp/src/contrib/graphbuilder.cpp
  contrib/yaml-cpp/src/conversion.cpp
  contrib/yaml-cpp/src/directives.cpp
  contrib/yaml-cpp/src/emitfromevents.cpp
  contrib/yaml-cpp/src/emitter.cpp
  contrib/yaml-cpp/src/emitterstate.cpp
  contrib/yaml-cpp/src/emitterutils.cpp
  contrib/yaml-cpp/src/exp.cpp
  contrib/yaml-cpp/src/iterator.cpp
  contrib/yaml-cpp/src/nodebuilder.cpp
  contrib/yaml-cpp/src/node.cpp
  contrib/yaml-cpp/src/nodeownership.cpp
  contrib/yaml-cpp/src/null.cpp
  contrib/yaml-cpp/src/ostream.cpp
  contrib/yaml-cpp/src/parser.cpp
  contrib/yaml-cpp/src/regex.cpp
  contrib/yaml-cpp/src/scanner.cpp
  contrib/yaml-cpp/src/scanscalar.cpp
  contrib/yaml-cpp/src/scantag.cpp
  contrib/yaml-cpp/src/scantoken.cpp
  contrib/yaml-cpp/src/simplekey.cpp
  contrib/yaml-cpp/src/singledocparser.cpp
  contrib/yaml-cpp/src/stream.cpp
  contrib/yaml-cpp/src/tag.cpp)

add_library(fncs SHARED
  src/echo.cpp
  src/fncs.cpp
  src/fncs_capi.cpp
  src/fncs_fapi.cpp)
target_link_libraries(fncs PRIVATE
 jsoncpp
 yamlcpp
 ${CZMQ_LIBRARY}
 ${LIBZMQ_LIBRARY})

add_executable(fncs_broker src/broker.cpp)
target_link_libraries(fncs_broker PRIVATE
 fncs
 ${CZMQ_LIBRARY}
 ${LIBZMQ_LIBRARY})

add_executable(fncs_player src/player.cpp)
target_link_libraries(fncs_player PRIVATE fncs)

add_executable(fncs_tracer src/tracer.cpp)
target_link_libraries(fncs_tracer PRIVATE fncs)

install(TARGETS fncs 
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
install(FILES src/fncs.h src/fncs.hpp DESTINATION include)

install(TARGETS fncs_broker DESTINATION bin)
install(TARGETS fncs_player DESTINATION bin)
install(TARGETS fncs_tracer DESTINATION bin)

