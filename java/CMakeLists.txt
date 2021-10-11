PROJECT( fncs-java )

CMAKE_MINIMUM_REQUIRED( VERSION 3.1 )

FIND_PACKAGE(Java COMPONENTS Development REQUIRED)
FIND_PACKAGE(JNI REQUIRED)
INCLUDE(UseJava)

# find fncs
INCLUDE(FindPackageHandleStandardArgs)
FIND_PATH(FNCS_INCLUDE_DIR fncs.hpp PATHS $ENV{FNCS_INCLUDE_DIR})
FIND_LIBRARY(FNCS_LIBRARY fncs PATHS $ENV{FNCS_LIBRARY})
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FNCS
    DEFAULT_MSG
    FNCS_INCLUDE_DIR
    FNCS_LIBRARY
)
MARK_AS_ADVANCED(FNCS_LIBRARY FNCS_INCLUDE_DIR)
IF(FNCS_FOUND)
    SET(FNCS_LIBRARIES    ${FNCS_LIBRARY})
    SET(FNCS_INCLUDE_DIRS ${FNCS_INCLUDE_DIR})
ELSE()
    MESSAGE (SEND_ERROR "fncs library not found")
ENDIF()

# generate JNIfncs.h stub
#ADD_CUSTOM_COMMAND(
#    OUTPUT fncs_JNIfncs.h
#    COMMAND ${Java_JAVAH_EXECUTABLE} -verbose
#        -classpath fncs.jar
#        -jni fncs.JNIfncs
#    MAIN_DEPENDENCY fncs.jar
#)
ADD_CUSTOM_COMMAND(
    OUTPUT fncs_JNIfncs.h
    COMMAND ${Java_JAVAC_EXECUTABLE} -h ../fncs -verbose
        -classpath fncs
         ../fncs/JNIfncs.java
    MAIN_DEPENDENCY fncs.jar
)

ADD_JAR(
    fncs
    fncs/JNIfncs.java
)

# generate fncs JNI module
INCLUDE_DIRECTORIES(
    ${JAVA_INCLUDE_PATH}
    ${JAVA_INCLUDE_PATH2}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${FNCS_INCLUDE_DIRS}
)
ADD_LIBRARY(JNIfncs MODULE fncs/JNIfncs.cpp fncs_JNIfncs.h)
TARGET_LINK_LIBRARIES(JNIfncs ${JNI_LIBRARIES} ${FNCS_LIBRARIES})
## The library name expected by the JNI relative to the system:
##   MacOS  : libLIBRARY.jnilib
##   Windows: LIBRARY.dll
##   Linux  : libLIBRARY.so
##
## where LIBRARY is used in java to load the library:
##      System.loadLibrary("LIBRARY");
IF(APPLE)
  SET_TARGET_PROPERTIES(JNIfncs PROPERTIES PREFIX "lib" SUFFIX ".jnilib")
ELSEIF(WIN32)
  SET_TARGET_PROPERTIES(JNIfncs PROPERTIES PREFIX "" SUFFIX ".dll")
ELSEIF(UNIX)
  SET_TARGET_PROPERTIES(JNIfncs PROPERTIES PREFIX "lib" SUFFIX ".so")
ENDIF(APPLE)

ENABLE_TESTING()

# add test to run JNIfncs
ADD_TEST(NAME TestJNIfncs
    COMMAND ${Java_JAVA_EXECUTABLE}
    -cp fncs.jar fncs.JNIfncs)
