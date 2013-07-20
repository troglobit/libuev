libuev - Asynchronous event loop library
========================================

The libuev library is a small event library in the style of the venerable
Xt(3) event loop.

Basically an event context is created,and handlers for timeounts and stream
events can be registered with this context.

After initial setup, the typical application will enter the event loop that
will only return at program termination.

The C API
---------
The typical application will do the following steps

1. Create the event context with uev_ctx_create()
2. Register callbacks with uev_io_create() and uev_timer_create()
3. Enter the event loop with uev_run()

Building
--------
Use make:

* make all: The library
* make test: Test and showcase

Installation
------------
Is purely manual, but would be something along the lines of:
 sudo install -m 444 libuev.so.1 /usr/lib/libuev.so.1

Warning
-------
Not thread safe!

The ctxt structure needs mutex protection in case of multi threaded
access.

