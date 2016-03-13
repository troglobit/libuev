libuEv | Simple event loop for Linux
====================================
[![Travis Status][]][Travis] [![Coverity Status][]][Coverity Scan]


Table of Contents
-----------------

* [Introduction](#introduction)
* [API](#api)
  * [Create an Event Context](#create-an-event-context)
  * [Register an Event Watcher](#register-an-event-watcher)
  * [Start Event Loop](#start-event-loop)
  * [Summary](#summary)
* [Joystick Example](#joystick-example)
* [Build & Install](#build--install)
* [Origin & References](#origin--references)


Introduction
------------

> “Why an event loop, why not use threads?”

With the advent of light-weight processes (threads) programmers these
days have a [golden hammer](http://c2.com/cgi/wiki?GoldenHammer) they
often swing without consideration.  Event loops and non-blocking I/O is
often a far easier approach, as well as less error prone.

The purpose of many applications is, with a little logic sprinkled on
top, to act on network packets entering an interface, timeouts expiring,
mouse clicks, or other types of events.  Such applications are often
very well suited to use an event loop.

Applications that need to churn massively parallel algorithms are more
suitable for running multiple (independent) threads on several CPU
cores.  However, threaded applications must deal with the side effects
of concurrency, like race conditions, deadlocks, live locks, etc.
Writing error free threaded applications is hard, debugging them can be
even harder.

Sometimes the combination of multiple threads *and* an event loop per
thread can be the best approach, but each application of course needs to
be broken down individually to find the most optimal approach.  Do keep
in mind, however, that not all systems your application will run on have
multiple CPU cores -- some small embedded systems still use a single CPU
core, even though they run Linux, with multiple threads a program may
actually run slower!  Always profile your program, and if possible, test
it on different architectures.

[libuEv][] is a simple event loop in the style of the more established
[libevent][1], [libev][2] and the venerable [Xt(3)][3] event loop.  The
*u* (micro) in the name refers to both the small feature set and the
small size overhead impact of the library.  The primary target of
[libuEv][] is a modern Linux system.

Experienced developers may appreciate that [libuEv][] is built on top of
modern Linux APIs: epoll, timerfd and signalfd.  Note, a certain amount
of care is needed when dealing with APIs that employ signalfd.  For
details, see [this article][4] at [lwn.net](http://lwn.net).

> “Event driven software improves concurrency” -- [Dave Zarzycki, Apple]


API
---

The C interface to [libuEv][] is very simple.  It handles three different
types of events: I/O (files, sockets, message queues, etc.), timers, and
signals.  With a slight caveat on signals

```C

    /* Callback example, arg is passed from watcher's *_init()
     * w->fd holds the file descriptor, events is set by libuEv
     * to indicate if any of UEV_READ and/or UEV_WRITE is ready.
     */
    void callback       (uev_t *w, void *arg, int events);

    /* Event loop:      Notice the use of flags! */
    int uev_init        (uev_ctx_t *ctx);
    int uev_exit        (uev_ctx_t *ctx);
    int uev_run         (uev_ctx_t *ctx, int flags);         /* UEV_NONE, UEV_ONCE, and/or UEV_NONBLOCK */
    
    /* I/O watcher:     fd is non-blocking, events is UEV_READ and/or UEV_WRITE */
    int uev_io_init     (uev_ctx_t *ctx, uev_t *w, uev_cb_t *cb, void *arg, int fd, int events);
    int uev_io_set      (uev_t *w, int fd, int events);
    int uev_io_start    (uev_t *w);
    int uev_io_stop     (uev_t *w);
    
    /* Timer watcher:   timeout and period in milliseconds */
    int uev_timer_init  (uev_ctx_t *ctx, uev_t *w, uev_cb_t *cb, void *arg, int timeout, int period);
    int uev_timer_set   (uev_t *w, int timeout, int period); /* Change timeout or period */
    int uev_timer_start (uev_t *w);                          /* Restart a stopped timer */
    int uev_timer_stop  (uev_t *w);                          /* Stop a timer */
    
    /* Signal watcher:  signo is the signal to wait for, e.g., SIGTERM */
    int uev_signal_init (uev_ctx_t *ctx, uev_t *w, uev_cb_t *cb, void *arg, int signo);
    int uev_signal_set  (uev_t *w, int signo);               /* Change signal to wait for */
    int uev_signal_start(uev_t *w);                          /* Restart a stopped signal watcher */
    int uev_signal_stop (uev_t *w);                          /* Stop signal watcher */

```

### Create an Event Context

To monitor events the developer first creates an *event context*, this
is achieved by calling `uev_init()` with a pointer to a (thread) local
`uev_ctx_t` variable.

```C

    uev_ctx_t ctx;
    
    uev_init(&ctx);

```


### Register an Event Watcher

For each event to monitor, be it a signal, timer or file descriptor, a
*watcher* must be registered with the event context.  The watcher, an
`uev_t`, is registered by calling the event type's `_init()` function
with the `uev_ctx_t` context, the callback, and an optional argument.

Here is a signal example:

```C

    void cleanup_exit(uev_t *w, void *arg, int events)
    {
        /* Graceful exit, with optional cleanup ... */
        uev_exit(w->ctx);
    }
    
    int main(void)
	{
	    uev_t sigterm_watcher;
        .
		.
		.
        uev_signal_init(&ctx, &sigterm_watcher, cleanup_exit, NULL, SIGTERM);
		.
		.
		.
	}

```


### Start Event Loop

When all watchers are registered, call the *event loop* with `uev_run()`
and the argument to the event context.  The `flags` parameter can be
used to integrate [libuEv][] into another event loop.

In this example we set `flags` to none:

```C

    uev_run(&ctx, UEV_NONE);

```

With `flags` set to `UEV_ONCE` the event loop returns as soon as it has
served the first event.  If `flags` is set to `UEV_ONCE | UEV_NONBLOCK`
the event loop returns immediately if no event is available.

**Note:** libuEv handles many types of errors, stream close, or peer
shutdowns internally, but also lets the callback run.  This is useful
for stateful connections to be able to detect EOF.

### Summary

1. Set up an event context with `uev_init()`
2. Register event callbacks with the event context using
   `uev_io_init()`, `uev_signal_init()` or `uev_timer_init()`
3. Start the event loop with `uev_run()`
4. Exit the event loop with `uev_exit()`, possibly from a callback

**Note 1:** Make sure to use non-blocking stream I/O!  Most hard to find
bugs in event driven applications are due to sockets and files being
opened in blocking mode.  Be careful out there!

**Note 2:** When closing a descriptor or socket, make sure to first stop
  your watcher, if possible.  This will help prevent any nasty side
  effects on your program.

**Note 3:** As mentioned above, a certain amount of care is needed when
dealing with signalfd.  This means that if your application, for
instance, uses `system()` you must redesign that to use `fork()`, and
then in the child, unblock all signals blocked by your parent process,
before you run `exec()`.  This because Linux does not unblock signals
for your children, and neither does most (all?) C-libraries.  An example
of this, from [finit][6], implementing `run()` as a better replacement
to `system()`, which sucks anyawy :)


Joystick Example
----------------

Here follows a very brief example to illustrate how one can use
[libuEv][] to act upon joystick input.

```C

    #include <err.h>
    #include <errno.h>
    #include <stdio.h>
    #include <stdint.h>
    #include <fcntl.h>
    #include <unistd.h>
    
    #include "uev.h"
    
    struct js_event {
    	uint32_t time;		/* event timestamp in milliseconds */
    	int16_t  value;		/* value */
    	uint8_t  type;		/* event type */
    	uint8_t  number;	/* axis/button number */
    } e;
    
    static void joystick_cb(uev_t *w, void *arg, int events)
    {
    	read (w->fd, &e, sizeof(e));
    
    	switch (e.type) {
    	case 1:
    		if (e.value) printf("Button %d pressed\n", e.number);
    		else 	     printf("Button %d released\n", e.number);
    		break;
    
    	case 2:
    		printf("Joystick axis %d moved, value %d!\n", e.number, e.value);
    		break;
    	}
    }
    
    int main(void)
    {
    	int       fd = open("/dev/input/js1", O_RDONLY, O_NONBLOCK);
    	uev_t     js1_watcher;
    	uev_ctx_t ctx;
    
    	if (fd < 0)
    		errx(errno, "Cannot find a joystick attached.");
    
    	uev_init(&ctx);
    	uev_io_init(&ctx, &js1_watcher, joystick_cb, NULL, fd, UEV_READ);
    
    	puts("Starting, press Ctrl-C to exit.");
    
    	return uev_run(&ctx, 0);
    }

```

To compile the program, save the code as `joystick.c` and call GCC with
`gcc -o joystick joystick.c io.c timer.c signal.c main.c` from this
directory, skips using a Makefile altogether.  Alternatively, call the
`Makefile` with <kbd>make joystick</kbd> from this directory.

More complete and relevant example uses of [libuEv][] is the FTP/TFTP
server [uftpd][5], and the Linux `/sbin/init` replacement [finit][6].
Both successfully employ [libuEv][].

Also see the `bench.c` program (<kbd>make bench</kbd> from within the
library) for [reference benchmarks][7] against [libevent][1] and
[libev][2].


Using -luev
-----------

LibuEv is by default installed as a library with a single include file.

    #include <uev/uev.h>

The output from the `pkg-config` tool holds no surprises:

    $ pkg-config --libs --static --cflags libuev
    -I/usr/local/include -L/usr/local/lib -luev

The prefix path `/usr/local/` used here is only the default.  Use the
`configure` script to select a different path when installing libuEv.


Build & Install
---------------

The library is built for and developed on GNU/Linux systems, patches to
support *BSD and its kqueue interface are most welcome.

libuEv use the GNU configure and build system.  To try out the bundled
examples, use the `--enable-examples` switch to the `configure` script.

    ./configure
    make all
    make test
    make install

The resulting .so file is ~14 kiB when stripped (`make install-strip`).


Origin & References
-------------------

[libuEv][] was originally based on [LibUEvent][8] by [Flemming Madsen][]
but has been completely rewritten with a much clearer API.  Now more
similar to the famous [libev][2] by [Mark Lehmann][].  Another small
event library used for inspiration is the very small [Picoev][9] by
[Oku Kazuho][].

[libuEv][] is developed and maintained by [Joachim Nilsson][].

[1]: http://libevent.org
[2]: http://software.schmorp.de/pkg/libev.html
[3]: http://unix.com/man-page/All/3x/XtDispatchEvent
[4]: http://lwn.net/Articles/415684/
[5]: https://github.com/troglobit/uftpd
[6]: https://github.com/troglobit/finit
[7]: http://libev.schmorp.de/bench.html
[8]: http://code.google.com/p/libuevent/
[9]: https://github.com/kazuho/picoev
[Travis]:          https://travis-ci.org/troglobit/libuev
[Travis Status]:   https://travis-ci.org/troglobit/libuev.png?branch=master
[Coverity Scan]:   https://scan.coverity.com/projects/3846
[Coverity Status]: https://scan.coverity.com/projects/3846/badge.svg
[LibuEv]:          https://github.com/troglobit/libuev
[Oku Kazuho]:      https://github.com/kazuho
[Mark Lehmann]:    http://software.schmorp.de
[Joachim Nilsson]: http://troglobit.com
[Flemming Madsen]: http://www.madsensoft.dk
[Dave Zarzycki, Apple]: http://www.youtube.com/watch?v=cD_s6Fjdri8

<!--
  -- Local Variables:
  --  mode: markdown
  -- End:
  -->
