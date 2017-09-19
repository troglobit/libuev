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
* [Motivation](MOTIVATION.md#background)
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


Documentation
-------------

The API documentation is available as a [separate document](API.md).


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

<!--
  -- Local Variables:
  --  mode: markdown
  -- End:
  -->
