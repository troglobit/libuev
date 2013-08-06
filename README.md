libuev - Micro event loop library
=================================

The libuev library is a small event library in the style of libevent,
the excellent libev and the venerable Xt(3) event loop.

It is heavily modelled after the libev API, but will never be as
portable to other UNIX systems.  The primary target of this project is
modern embedded Linux systems.  You will need kernel and C-library
support for epoll(7), timerfd and signalfd.


Usage
-----

The user initializes an event context to which timer and I/O stream
watchers are registered.  A typical application then enters the event
loop that will only return at program termination.


The C API
---------

The typical application will do the following steps

   1. Prepare an event context with `uev_init()`
   2. Register callbacks with `uev_io_init()` and `uev_timer_init()`
   3. Enter the event loop with `uev_run()`

**Note:** Make sure to use non-blocking stream I/O!


Build & Install
---------------

The library is built and developed for GNU/Linux systems, as such it may
use GNU GCC and GNU Make specific features.  Patches to support *BSD
kqueue are most welcome.

   * `make all`: The library
   * `make test`: Test and showcase
   * `make install`: Honors $prefix and $DESTDIR environment variables


Warning
-------

Please be advised, libuev is not thread safe!  It is primarily intended
for use-cases where the engineer wants to avoid the use of threads.  To
support threads the signal handling needs to be patched first, other
than that an event context per thread should be sufficient.

