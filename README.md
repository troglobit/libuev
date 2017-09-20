libuEv | Simple event loop for Linux
====================================
[![Travis Status][]][Travis] [![Coverity Status][]][Coverity Scan]


* [Introduction](#introduction)
* [API](API.md#api)
  * [Create an Event Context](API.md#create-an-event-context)
  * [Register an Event Watcher](API.md#register-an-event-watcher)
  * [Start Event Loop](API.md#start-event-loop)
  * [Summary](API.md#summary)
* [Using -luev](API.md#using--luev)
* [Joystick Example](API.md#joystick-example)
* [Build & Install](#build--install)
* [Origin & References](#origin--references)


> “Event driven software improves concurrency” -- [Dave Zarzycki, Apple][]

Introduction
------------

[libuEv][] is a simple event loop in the style of the more established
[libevent][1], [libev][2] and the venerable [Xt(3)][3] event loop.  The
*u* (micro) in the name refers to both the small feature set and the
small size overhead impact of the library.

Experienced developers may appreciate that [libuEv][] is built on top of
modern Linux APIs: epoll, timerfd and signalfd.


Example
-------

The library documentation is available as a [separate document](API.md).

```C
#include <stdio.h>
#include <uev/uev.h>

static void cb(uev_t *w, void *arg, int events)
{
	printf("Callback runs every other second.\n");
}

int main(void)
{
	uev_t timer;
	uev_ctx_t *ctx;

	uev_init(&ctx);
	uev_timer_init(&ctx, &timer, cb, NULL, 2 * 1000, 0);

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
make test
sudo make install-strip
sudo ldconfig
```

The resulting .so file is ~14 kiB.


Origin & References
-------------------

[libuEv][] was originally based on [LibUEvent][8] by [Flemming Madsen][]
but has been completely rewritten to provide a cleaner API.  It is now
more similar to the famous [libev][2] by [Mark Lehmann][].  Another
small event library used for inspiration is the very small [picoev][9]
by [Oku Kazuho][].

[libuEv][] is developed and maintained by [Joachim Nilsson][].  It is
built for and developed on GNU/Linux systems, patches to support *BSD
and its [kqueue][] interface are most welcome.


[1]: http://libevent.org
[2]: http://software.schmorp.de/pkg/libev.html
[3]: http://unix.com/man-page/All/3x/XtDispatchEvent
[8]: http://code.google.com/p/libuevent/
[9]: https://github.com/kazuho/picoev
[Travis]:          https://travis-ci.org/troglobit/libuev
[Travis Status]:   https://travis-ci.org/troglobit/libuev.png?branch=master
[Coverity Scan]:   https://scan.coverity.com/projects/3846
[Coverity Status]: https://scan.coverity.com/projects/3846/badge.svg
[LibuEv]:          https://github.com/troglobit/libuev
[kqueue]:          https://github.com/mheily/libkqueue
[Oku Kazuho]:      https://github.com/kazuho
[Mark Lehmann]:    http://software.schmorp.de
[Joachim Nilsson]: http://troglobit.com
[Flemming Madsen]: http://www.madsensoft.dk
[Dave Zarzycki, Apple]: http://www.youtube.com/watch?v=cD_s6Fjdri8
