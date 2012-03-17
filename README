LibUEvent - A Microscopic event library.
========================================

The libuevent library is a small event library in the style of the venerable
Xt(3) event loop.

Basically an event context is created,and handlers for timeounts and stream
events can be registered with this context.

After initial setup, the typical application will enter the event loop that
will only return at program termination.

The C API
---------
The typical application will do the following steps
1. Create the event context with lueCtxtCreate()
2. Register initial callbacks with lueAddInput, lueAddOutput and lueAddTimer
3. Enter the event loop

Lua binding
-----------
A lua binding is available that should mix well with luasocket and luaposix.
Time resolutions are in milli second integers in order to support integer-only
embedded environments

### function luevent.addFileIO(fdesc, handler, mode)
Add a handler on a stream.  The handler will sustain itself GC wise.
Use *object*:clear() to unregister the handler
* fdesc   The posix file descriptor "small integer".  
          Use the ":getfd()" method on a "normal" lua stream or socket to obtain fdesc.
* handler A function object. The handler object is supplied as the first parameter when called.
* mode    Optional. Input ("r") or output ("w") event. Default is input
* return  A new filehandler object

### function luevent.addTimer(msecs, handler)
Create a new timer object. Implicitly cleared when handler is called.
Typical usage:
  local hdl
  hdl = luevent.addTimer(50, function() hdl = nil ..... end)
  ......
  if hdl then hdl:clear() hdl = nil end
Here *hdl* is used to keep track of whether the timer is active.

* msecs   The number of milliseconds until the timer expires
* handler A function object. The handler object is supplied as the first parameter when called.
* return  A new timer object

### function object:clear()
Cancel a callback registration. Renders the object ready for GC.

### function object:handler()
Obtain the handler function from a callback registration.


Building
--------
Use make:

* make all: The library
* make test: Test and showcase
* make lua: Build the lua binding

Installation
------------
Is purely manual, but would be something along the lines of:
 sudo install -m 444 libuevent.so.1 /usr/lib/libuevent.so.1
 sudo install -m 444 luevent.so /usr/lib/lua/5.1/luevent.so

<!-- vim: set syntax=mkd textwidth=79 nofoldenable: -->

