libuev | Micro event loop library
=================================

Libuev is a small event library in the style of libevent, the excellent
libev and the venerable Xt(3) event loop.  The primary target is modern
embedded Linux systems.


Usage
-----

The user initializes an event context to which timer and I/O stream
watchers are registered.  A typical application then enters the event
loop that will only return at program termination.

   1. Prepare an event context with `uev_init()`
   2. Register callbacks with `uev_io_init()`, `uev_signal_init()` and
      `uev_timer_init()`
   3. Enter the event loop with `uev_run()`
   4. Exit the event loop with `uev_exit()`, possibly from a callback

**Note:** Make sure to use non-blocking stream I/O!


Example
-------

    #include "uev.h"
    
    static void signal_cb(uev_ctx_t *ctx, uev_t *w, void *UNUSED(arg), int UNUSED(events))
    {
	    printf("Received signal %d, exiting ...\n", w->signo);
        uev_exit(ctx);
    }
    
    static void read_cb(uev_ctx_t *UNUSED(ctx), uev_t *w, void UNUSED(*arg), int UNUSED(events))
    {
        char buf[80];

        read(w->fd, buf, sizeof(buf));
        /* Do stuff ... watcher restarted when callback exits */
    }
    
    int main(void)
    {
        int fd;
        uev_t io_watcher, sig_watcher;
        uev_ctx_t ctx;
    
        uev_init(&ctx);
    
        fd = open("/etc/passwd", O_RDONLY, O_NONBLOCK);
        uev_io_init(&ctx, &io_watcher, read_cb, NULL, fd, UEV_READ);

        uev_signal_init(&ctx, &sig_watcher, signal_cb, NULL, SIGINT);

        return uev_run(&ctx);
    }


API
---

    int uev_init        (uev_ctx_t *ctx);
    int uev_exit        (uev_ctx_t *ctx);
    int uev_run         (uev_ctx_t *ctx, int flags);
    
    int uev_io_init     (uev_ctx_t *ctx, uev_t *w, uev_cb_t *cb, void *arg, int fd, int events);
    int uev_io_set      (uev_t *w, int fd, int events);
    int uev_io_stop     (uev_t *w);
    
    int uev_timer_init  (uev_ctx_t *ctx, uev_t *w, uev_cb_t *cb, void *arg, int timeout, int period);
    int uev_timer_set   (uev_t *w, int timeout, int period);
    int uev_timer_stop  (uev_t *w);
    
    int uev_signal_init (uev_ctx_t *ctx, uev_t *w, uev_cb_t *cb, void *arg, int signo);
    int uev_signal_set  (uev_t *w, int signo);
    int uev_signal_stop (uev_t *w);


Build & Install
---------------

The library is built and developed for GNU/Linux systems, as such it may
use GNU GCC and GNU Make specific features.  Patches to support *BSD
kqueue are most welcome.

   * `make all`: The library
   * `make test`: Test and showcase
   * `make install`: Honors `$prefix` and `$DESTDIR` environment variables

Size of libuev:

    $ make strip
    CC      main.o
    CC      io.o
    CC      timer.o
    CC      signal.o
    ARCHIVE libuev.a
    LINK    libuev.so.1
    STRIP   libuev.a
    STRIP   libuev.so.1
    text	   data	    bss	    dec	    hex	filename
    1177	      0	      0	   1177	    499	main.o     (ex libuev.a)
     308	      0	      0	    308	    134	io.o       (ex libuev.a)
     682	      0	      0	    682	    2aa	timer.o    (ex libuev.a)
     563	      0	      0	    563	    233	signal.o   (ex libuev.a)
    6306	    768	      8	   7082	   1baa	libuev.so.1


Caveat
------

Please be advised, libuev is not thread safe!  It is primarily intended
for developers who want to entirely avoid using threads.  To support
threads the signal handling needs to be patched first, other than that
an event context per thread should be sufficient.


Origin & References
--------------------

Libuev was originally based on
[LibUEvent](http://code.google.com/p/libuevent/) by
[Flemming Madsen](http://www.madsensoft.dk/) but has been completely
rewritten and is now more similar to the famous
[libev](http://software.schmorp.de/pkg/libev.html) by Mark Lehmann.
Another small event library used for inspiration is the very small
[Picoev](https://github.com/kazuho/picoev) by
[Oku Kazuho](https://github.com/kazuho).

   * http://code.google.com/p/libuevent/
   * http://software.schmorp.de/pkg/libev.html
   * http://libev.schmorp.de/bench.html
   * http://libevent.org/
   * http://developer.cybozu.co.jp/archives/kazuho/2009/08/picoev-a-tiny-e.html
   * http://coderepos.org/share/browser/lang/c/picoev/

Libuev is maintained by [Joachim Nilsson](mailto:troglobit@gmail.com) at
[GitHub](https://github.com/troglobit/libuev)

