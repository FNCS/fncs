# Framework for Network Co-Simulation 2

Version 2 of FNCS is simplifying and extending [the original FNCS](https://github.com/GridOPTICS/FNCS), focusing on the needs to support the development of control algorithms across diverse simulator software packages and hardware.

## How to Install FNCS (Linux)

The FNCS code is available at https://github.com/GridOPTICS/FNCS2.  FNCS depends on two other software libraries,

 1. [ZeroMQ](http://zeromq.org/), and
 2. its higher-level C binding [CZMQ](http://czmq.zeromq.org/).

In order, the dependencies are linear; build ZeroMQ, then CZMQ, then FNCS.

These steps assume your OS is Linux and you are using a Bash shell.  We will be installing all software into a subdirectory of your home directory, i.e., $HOME/FNCS_install.

### ZeroMQ

```bash
# we are doing everything from your $HOME directory
cd $HOME

# download zeromq
wget http://download.zeromq.org/zeromq-3.2.4.tar.gz
# if you do not have wget, use
# curl -O http://download.zeromq.org/zeromq-3.2.4.tar.gz

# unpack zeromq, change to its directory
tar -xzf zeromq-3.2.4.tar.gz
cd zeromq-3.2.4

# configure, make, and make install 
./configure --prefix=$HOME/FNCS_install
make
make install
```

### CZMQ

Installing CZMQ is like any other software using configure and make.  The main challenge is specifying the installation location (--prefix) for CZMQ as well as the location where ZeroMQ was installed.  If you installed ZeroMQ as written above, the following will work for you.

```bash
# we are doing everything from your $HOME directory
cd $HOME

# download czmq
wget http://download.zeromq.org/czmq-3.0.0-rc1.tar.gz
# if you do not have wget, use
# curl -O http://download.zeromq.org/czmq-3.0.0-rc1.tar.gz

# unpack czmq, change to its directory
tar -xzf czmq-3.0.0-rc1.tar.gz
cd czmq-3.0.0

# configure, make, and make install 
./configure --prefix=$HOME/FNCS_install --with-libzmq=$HOME/FNCS_install
make
make install
```

### FNCS

Installing FNCS2 is like any other software using configure and make.  The main challenge is specifying the installation location (--prefix) for FNCS2 as well as the location where ZeroMQ and CZMQ were installed.  If you installed ZeroMQ and CZMQ as written above, the following will work for you.

Please note that you must use your own username to clone the git repository as indicated by <USER> below. If while attempting to clone FNCS from the git repository, you get an error "server certificate verification failed," check here for what to do: http://stackoverflow.com/questions/21181231/server-certificate-verification-failed-cafile-etc-ssl-certs-ca-certificates-c.

```bash
# we are doing everything from your $HOME directory
cd $HOME

# download FNCS
git clone https://github.com/GridOPTICS/FNCS2.git

# change to FNCS directory
cd FNCS2

# configure, make, and make install 
./configure --prefix=$HOME/FNCS_install --with-zmq=$HOME/FNCS_install
make
make install
```

## How to Run a FNCS Co-Simulation

FNCS co-simulations depend on a server process called the `fncs_broker`. The simulatiors connect to the broker. Run the installed FNCS broker application and indicate the number of simulators that will connect.  This number can be 1 as is sometimes useful for testing.

`./fncs_broker 2`

Then run a FNCS-capable simulator.

`./fncs_player 10m trace.txt`

## How to Use FNCS Tracer/Player Simulators

When wanting to debug a FNCS-ready simulator in isolation, i.e., without other complex FNCS-ready simulators, it is useful to deploy a tracer and player simulator. The tracer simulator by default will subscribe to all message types and write a trace file.  The trace file can then be given to a player simulator to play back the events that occurred. The tracer is a good tool to make sure your simulator is actually publishing values. The player is a good tool to make sure your simulator is receiving published values.

### Tracer/Player File Format

```
# nanoseconds   topic                   value
1               sim1/key                value
1               sim1/objA:objB/key      value
1               sim2/yet                again
10000000001     sim3/key                value2
15000000001     sim1/key2               value
```

#### Comments

The file format begins with zero or more optional comments.  Comments must begin with the hash (#) and the hash must be placed as the first character of the line.

#### Events

Each line in the file represents an event.  The line is split into three columns based on any arbitrary whitespace except newline.

 1. Time at which the event occurred.
 2. Topic name for the event.
 3. Value of the event.

Events MUST be in increasing sorted order according to the time field.  Events are allowed to occur simultaneously, as indicated by the events occurring at 1 nanosecond in the above example.

#### How Time is Handled

The "time" column is intentionally without units.  The comment line used in the example above is not mandatory, rather it is useful to indicate the intended unit for time.  A time unit may be enforced later by a command-line parameter given to the FNCS player.

```bash
./fncs_player 10m trace.txt
# 10m is 10 minutes
# FNCS recognizes many different time unit strings
```

### FNCS ZPL Config File

The ZeroMQ Property Language (ZPL) defines a minimalistic framing language for specifying property sets, expressed as a hierarchy of name-value property pairs. 
Details about the ZPL format can be found at the ZeroMQ website.

http://rfc.zeromq.org/spec:4

#### How to Use the FNCS ZPL Config File

Each simulator should have a corresponding "fncs.zpl" file.  By default, the fncs.zpl is assumed to be in the current working directory where the simulator is launched.  Otherwise, you can specify the name and/or location of the file using the environment variable FNCS_CONFIG_FILE.

#### Example fncs.zpl

The following code block is an example of the fncs.zpl file.  The inline comments in the code block indicate which fields are required.  In short, the only required fields are the name, time_delta, and broker address.  The subscriptions are actually optional (think of a weather simulator that only reports the temperature and ignores all others).  However, if you specify a subscription, each subscription has some required values and some optional values.

```bash
name = sim1                 # required; across the co-simulation, all names must be unique
time_delta = 1s             # required; format is <number><unit>; smallest time step supported by the simulator
broker = tcp://localhost:5570   # required; broker location
values                      # optional; list of exact-string-matching topic subscriptions
    foo                     # required; lookup key
        topic = some_topic  # required; format is any reasonable string (not a regex)
        default = 10        # optional; default value
        type = int          # optional; currently unused; data type
        list = false        # optional; defaults to "false"; whether incoming values queue up (true) or overwrite the last value (false)
    bar                     # see "foo" above
        topic = some_topic  # see "foo" above
        default = 0.1       # see "foo" above; here we used a floating point default
        type = double       # see "foo" above
        list = true         # see "foo" above; this is the only difference between "foo" and "bar" here
matches                     # optional; list of regular-expression matching topic subscriptions
    baz                     # required; lookup key
        topic = sim1/.*     # required; topic (a regex)
```

##### Values

The list of exact-string-matching topic subscriptions is intended to model a list of simple key-value pairs.  Think of your simulator code and its variables - each variable has a name and its associated value.  That is how you would write the list of "values" in the FNCS ZPL file as well as how you would retrieve values at runtime using the string `fncs::get_value(string key)` or the `vector<string> fncs::get_values(string key)` functions.  In most cases each subscription is for a single value (or array of values perhaps).  In some cases, a reduction operation is useful such as when computing a sum of values from individual publishers – we need the values to queue up rather than have the last value overwrite all the others.

##### Matches

The list of regular-expression matching topic subscriptions is considerably different from a string-matching topic.  Different from "values", the matches always queue up.  In addition, since the regular expression may include wildcards such as '*' or '.' or '?', the exact topic that was matched is not known until runtime.  The actual topic string received at runtime is valuable information and as such is returned to the caller of `vector<pair<string,string>> fncs::get_matches(string key)`.  Formally, the function returns a vector of string pairs, with the pair being the matched topic and the associated value.

### Environment Variables

|Variable           |Default Value          |Description                                                                                |
|-------------------|-----------------------|-------------------------------------------------------------------------------------------|
|FNCS_LOG_FILE      |fncs.log               |File where log messages go.  Currently echoed to stdout as well as this file.              |
|FNCS_CONFIG_FILE   |fncs.zpl               |File where configuration stuff goes.                                                       |
|FNCS_NAME          |N/A                    |Same meaning as what is in the ZPL file. Name of the simulator. Must be globally unique.   |
|FNCS_BROKER\*      |tcp://localhost:5570   |Same meaning as what is in the ZPL file. Location of broker endpoint.                      |
|FNCS_TIME_DELTA    |N/A                    |Same meaning as what is in the ZPL file.                                                   |

\* If this environment variable is used with the fncs_broker application, it is best to specify tcp://*:PPPP where PPPP is the port number. If this environment variable is used with a FNCS-ready application, it is best to specify tcp://hostname:PPPP where hostname is the name of the host, e.g., localhost, and PPPP is the port number.
