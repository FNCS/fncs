INSTALL_PATH := ~/src/tesp/examples/loadshed/java
FNCS_PATH := /usr/local
CXX = g++

CXXFLAGS =
CXXFLAGS += -O2
CXXFLAGS += -std=c++0x

CPPFLAGS =
CPPFLAGS += -I$(FNCS_PATH)/include
CPPFLAGS += -I/usr/local/include
CPPFLAGS += -I/usr/lib/jvm/default-java/include
CPPFLAGS += -I/usr/lib/jvm/default-java/include/linux
CPPFLAGS += -I./fncs
CPPFLAGS += -fPIC

LDFLAGS =
LDFLAGS += -L$(FNCS_PATH)/lib

LIBS =
LIBS += -lfncs
LIBS += -lczmq
LIBS += -lzmq

all: fncs.jar libJNIfncs.so

fncs.jar: fncs/JNIfncs.java
	javac -h fncs fncs/JNIfncs.java
	jar cvf fncs.jar fncs/JNIfncs.class

libJNIfncs.so: fncs/JNIfncs.cpp
	$(CXX) $(CPPFLAGS) -DJNIfncs_EXPORTS -o fncs/JNIfncs.o -c fncs/JNIfncs.cpp
	$(CXX) -shared -o libJNIfncs.so fncs/JNIfncs.o -L. $(LIBS)

clean:
	rm -f fncs/*.class
	rm -f fncs/*.h
	rm -f fncs/*.o
	rm -f *.jar
	rm -f *.so

install:
	cp fncs.jar $(INSTALL_PATH)
	cp libJNIfncs.so $(INSTALL_PATH) 
