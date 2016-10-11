# Messaging Benchmark A

This benchmark features two types of simulators, "T" and "D", with a single instance of "T" exchanging data with one or more "D"s.  Data is exchanged once per second.  The "T" simulator operates with millisecond precision while the "D" simulators operate at second precision.

"T" will send two values of 8 bytes each every tick.

Each "D" will send two values of 8 bytes each every tick.