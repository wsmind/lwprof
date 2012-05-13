lwprof
======

lwprof is a lightweight time profiler for C++. It is designed to answer basic questions,
such as:
* How long does this piece of code take to execute?
* Is my parallel code actually faster?

It is _not_ a complex solution with hardware requirements or full-fledged graphical tools,
but just a simple way to get performance information out of your code.

Getting started
---------------

To install lwprof, just copy [lwprof.hpp](https://raw.github.com/wsmind/lwprof/master/src/lwprof.hpp)
and [lwprof.cpp](https://raw.github.com/wsmind/lwprof/master/src/lwprof.cpp) to your project. You
will need to include lwprof.hpp from the files you want to profile.

API
---

lwprof is an intrusive profiler. That means you must tag the parts of your code you want to measure.

// to be written
