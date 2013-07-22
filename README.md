libuev - Micro event loop library
=================================

The libuev library is a small event library in the style of libevent,
the excellent libev and the venerable Xt(3) event loop.

It is heavily modelled after the libev API, but will never be as
portable or friendly to older systems.  The focus of this project is
modern embedded Linux systems, you will need kernel and C-library
support for epoll(7) and timerfd.


Usage
-----

Basically an event context is created to which timer and I/O stream
watchers are registered.  After initial setup, the typical application
enters the event loop that will only return at program termination.


The C API
---------

The typical application will do the following steps

   1. Create the event context with `uev_ctx_create()`
   2. Register callbacks with `uev_io_create()` and `uev_timer_create()`
   3. Enter the event loop with `uev_run()`

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

