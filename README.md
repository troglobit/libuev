libuEv | Simple event loop for Linux
====================================
[![Travis Status][]][Travis] [![Coverity Status][]][Coverity Scan]


* [Introduction](#introduction)
* [API](#api)
  * [Create an Event Context](#create-an-event-context)
  * [Register an Event Watcher](#register-an-event-watcher)
  * [Start Event Loop](#start-event-loop)
  * [Summary](#summary)
* [Using -luev](#using--luev)
* [Joystick Example](#joystick-example)
* [Build & Install](#build--install)
* [Background](#background)
* [Origin & References](#origin--references)


Introduction
------------

[libuEv][] is a simple event loop in the style of the more established
[libevent][1], [libev][2] and the venerable [Xt(3)][3] event loop.  The
*u* (micro) in the name refers to both the small feature set and the
small size overhead impact of the library.

Experienced developers may appreciate that [libuEv][] is built on top of
modern Linux APIs: epoll, timerfd and signalfd.  Note however, a certain
amount of care is needed when dealing with APIs that employ signalfd.
For details, see [this article][4] at [lwn.net](http://lwn.net).

> “Event driven software improves concurrency” -- [Dave Zarzycki, Apple][]


Documentation
-------------

The API documentation is available as separate [README][] in the `src/`
directory.


Build & Install
---------------

The library is built for and developed on GNU/Linux systems, patches to
support *BSD and its [kqueue][] interface are most welcome.

libuEv use the GNU configure and build system.  To try out the bundled
examples, use the `--enable-examples` switch to the `configure` script.

```sh
    $ ./configure
    $ make -j5
    $ make test
    $ sudo make install-strip
    $ sudo ldconfig
```

The resulting .so file is ~14 kiB (<kbd>make install-strip</kbd>).


Background
----------

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
[README]:          https://github.com/troglobit/libuev/src/README.md
[kqueue]:          https://github.com/mheily/libkqueue
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
