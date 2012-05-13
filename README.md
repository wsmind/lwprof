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
The amount of code you need to add before seeing some time figures, however, is minimal.

The API is made of 3 macros.

### PROFILE_THREAD("Thread name")
This macro must be called from each thread that will call PROFILE_BLOCK (even if the application is single-threaded).
It will create a profiling context for the calling thread.
Note: the thread name must be a compile-time constant.

### PROFILE_BLOCK("Block name")
When added to a block, this macro will measure time elapsed between the start and end of the block. It
is the most useful thing in the API, the one actually measuring time.
Note: the block name must be a compile-time constant.

### PROFILE_DUMP(logger, js)
When you're done collecting profiling information, call PROFILE_DUMP to export the results. The logger
must be anything (function or object) that can be called with the parenthesis operator with a const char * as argument.
js is meant to output JSON, but this is WIP currently, so leave it to false ;)

Examples
--------

For a simple example of file IO profiling, see [fileio.cpp](https://raw.github.com/wsmind/lwprof/master/test/fileio.cpp).

[multithread.cpp](https://raw.github.com/wsmind/lwprof/master/test/multithread.cpp) exposes a more advanced usage, showing
how to profile multiple worker threads.
