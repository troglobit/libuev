libuev - Micro event loop library
=================================

The libuev library is a small event library in the style of the
venerable Xt(3) event loop.

Basically an event context is created and handlers for timers and I/O
stream events can be registered with this context.

After initial setup, the typical application will enter the event loop
that will only return at program termination.


The C API
---------

The typical application will do the following steps

   1. Create the event context with `uev_ctx_create()`
   2. Register callbacks with `uev_io_create()` and `uev_timer_create()`
   3. Enter the event loop with uev_run()

**Note:** Make sure to use non-blocking stream I/O!


Build & Install
---------------

The library is built and developed for GNU/Linux systems, as such it may
use GNU GCC and GNU Make specific features, but may also work with other
UNIX systems as well.

   * `make all`: The library
   * `make test`: Test and showcase
   * `make install`: Honors $prefix and $DESTDIR environment variables


Warning
-------

Please be advised, libuev is not thread safe!  It is primarily intended
for use-cases where the engineed wants to avoid the use of threads.

However, if you insist on trying to use libuev with threads, the `uev_t`
structure needs mutex protection against simultaneous access.

