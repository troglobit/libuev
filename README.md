µEv | Simple event loop for Linux
==================================
[![Travis Status][]][Travis] [![Coverity Status][]][Coverity Scan]


* [Introduction](#introduction)
* [API](API.md#overview)
  * [Create an Event Context](API.md#create-an-event-context)
  * [Register an Event Watcher](API.md#register-an-event-watcher)
  * [Start Event Loop](API.md#start-event-loop)
  * [Summary](API.md#summary)
* [Using -luev](API.md#using--luev)
* [Joystick Example](API.md#joystick-example)
* [Build & Install](#build--install)
* [Origin & References](#origin--references)


> **NOTE:** Incompatible failure mode changes in v2.0 compared to v1.x!

Introduction
------------

[libuEv][] is a small event loop that wraps the Linux `epoll()` family
of APIs.  It is similar to the more established [libevent][], [libev][]
and the venerable [Xt(3)][] event loop.  The *µ* in the name refers to
both its limited feature set and the size impact of the library.

Failure mode changes introduced in v2.0 may affect users of v1.x, See
the [ChangeLog][] for the full details.

The [API documentation](API.md) is available as a separate document.


Example
-------

Notice how watcher conditions like `UEV_ERROR` must be handled by each
callback.  I/O watchers must also check for `UEV_HUP`.  Both errors are
usually fatal, libuEv makes sure to stop each watcher before a callback
runs, leaving it up to the callback to take appropriate action.

```C
/* Set up a timer watcher to call cb() every other second */
#include <stdio.h>
#include <uev/uev.h>

static void cb(uev_t *w, void *arg, int events)
{
        if (UEV_ERROR == events) {
            puts("Problem with timer, attempting to restart.");
            uev_timer_start(w);
            return;
        }

        puts("Every other second");
}

int main(void)
{
        uev_ctx_t ctx;
        uev_t timer;

        uev_init(&ctx);
        uev_timer_init(&ctx, &timer, cb, NULL, 2 * 1000, 2 * 1000);

        return uev_run(&ctx, 0);
}
```


Build & Install
---------------

libuEv use the GNU configure and build system.  To try out the bundled
examples, use the `--enable-examples` switch to the `configure` script.
There is also a limited unit test suite that can be useful to learn how
the library works.

```sh
./configure
make -j5
make check
sudo make install-strip
sudo ldconfig
```

The resulting .so library is ~23 kiB.

To build from GIT sources; clone the repository and run the `autogen.sh`
script.  This requires GNU `automake`, `autoconf` amd `libtool` to be
installed on your system.  (If you build from a released tarball you do
not need them.)


Origin & References
-------------------

[libuEv][] is developed and maintained by [Joachim Wiberg][] on GitHub.
It is primarily built for and developed on GNU/Linux systems, patches to
support the BSD [kqueue][] interface are most welcome.

Originally based on [LibUEvent][] by [Flemming Madsen][], uEv has since
evolved to support all of the Linux `epoll()` family APIs.  It is now
more similar to the excellent [libev][] by [Mark Lehmann][], with some
inspiration also from [picoev][] by [Oku Kazuho][].


[ChangeLog]:       https://github.com/troglobit/libuev/blob/master/ChangeLog.md
[Travis]:          https://travis-ci.org/troglobit/libuev
[Travis Status]:   https://travis-ci.org/troglobit/libuev.png?branch=master
[Coverity Scan]:   https://scan.coverity.com/projects/3846
[Coverity Status]: https://scan.coverity.com/projects/3846/badge.svg
[libevent]:        https://libevent.org
[Xt(3)]:           https://unix.com/man-page/All/3x/XtDispatchEvent
[LibUEvent]:       https://code.google.com/p/libuevent/
[picoev]:          https://github.com/kazuho/picoev
[libev]:           http://software.schmorp.de/pkg/libev.html
[LibuEv]:          https://github.com/troglobit/libuev
[kqueue]:          https://github.com/mheily/libkqueue
[Oku Kazuho]:      https://github.com/kazuho
[Mark Lehmann]:    http://software.schmorp.de
[Joachim Wiberg]: http://troglobit.com
[Flemming Madsen]: http://www.madsensoft.dk
