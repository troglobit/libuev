Change Log
==========

All notable changes to the project are documented in this file.


[v1.2.0] - 2015-06-09
---------------------

API change in event callbacks and fix timers that never start.

### Changes
- Remove first `uev_ctx_t *` argument in callbacks, incompatible API
  change!  Please update all your callbacks if you upgrade.

### Fixes
- Fix timers that accidentally broke in [v1.1.0].
- Fixes to `bench.c`, it now actually listens to the pipe/socket.


[v1.1.0] - 2015-03-04
---------------------

Massively improved error handling.

### Changes
- Handle case when user closes a descriptor *before* stopping a watcher.
- Handle `EPOLLHUP` and `EPOLLERR`.  Restart `epoll(7)` descriptor and
  all watchers when an error count reaches a MAX value -- handles stale
  descriptors or cases when kernel does not notice updated descriptors.
- Return error when stopping a watcher fails.
- Update [README.md] with new `uev_*_start()` functions.
- Bump dev version to 1.1 due to the number of significant changes.

### Fixes
- Remove `test.c` from `DISTFILES` in `Makefile`.  You need the
  comeplete sources to build the examples now.  Thanks to @karasz for
  the heads up on this and the musl libc issue with missing `queue.h`!
- Fix broken link to [v1.0.5] in this file.


[v1.0.5] - 2015-02-15
---------------------

### Changes
- Add `uev_*_start()` functions.
- Add slightly odd `examples/signal.c` example that utilises `fork()`
  and causes segfault in child.
- Renamed `main.c` to `uev.c`
- Move examples to `examples/` subdirectory
- Simplify automatic dependency calculation
- Add [TODO.md] for wishlist items
- Add [CHANGELOG.md], attempt to align with http://keepachangelog.com
- Further updates to [README.md]


[v1.0.4] - 2015-01-24
---------------------

Minor documentation and build fixes.

Release mainly targeted for [Finit development](https://github.com/troglobit/finit/).

### Changes
* `test.c` has been simplified/clarified
* Further updates to [README.md]
* Minor update to Makefile, change how build progress is echoed


[v1.0.3] - 2015-01-24
---------------------

This is a very minor release, with a strong focus on documentation. 

### Changes
- [README.md], massively updated
- API documentation, updated
- Travis CI integration added, https://travis-ci.org/troglobit/libuev
- Coverity Scan integration, https://scan.coverity.com/projects/3846
- Clang scan-build support added


[v1.0.2] - 2015-01-07
---------------------

### Fixes
- Fix broken `make dist` target in [v1.0.1].


[v1.0.1] - 2015-01-07
---------------------

### Changes
- Cleanup and rewrite of [README.md] after first audit by @vonj
- Added API and example section to [README.md]
- New [joystick.c] example added
- `main.c:uev_run()`: Document mysterious flags parameter


[v1.0.0] - 2013-08-06
---------------------

First release in the new guise and API.

### Changes
- Add support for timers, using `timerfd_*()` and `epoll()` API's
- Add support for signals using `signalfd()` API
- Convert to use BSD `sys/queue.h` API in GLIBC instead of homebrew
  linked list implementation
- Massive changes to code structure, API naming
- Reindent to use Linux KNF
- Change library name to libuev
- Lua support removed
- Make sure to state the [MIT license] correctly in all files
- Import [Niels Provos]' `bench.c`, the [libev version]


v0.0.1 - 2012-03-17
-------------------

[Initial announcement] of [libuevent] by [Flemming Madsen] to the Lua
users mailing list.

[UNRELEASED]: https://github.com/troglobit/libuev/compare/v1.2.0...HEAD
[v1.2.0]: https://github.com/troglobit/libuev/compare/v1.1.0...v1.2.0
[v1.1.0]: https://github.com/troglobit/libuev/compare/v1.0.5...v1.1.0
[v1.0.5]: https://github.com/troglobit/libuev/compare/v1.0.4...v1.0.5
[v1.0.4]: https://github.com/troglobit/libuev/compare/v1.0.3...v1.0.4
[v1.0.3]: https://github.com/troglobit/libuev/compare/v1.0.2...v1.0.3
[v1.0.2]: https://github.com/troglobit/libuev/compare/v1.0.1...v1.0.2
[v1.0.1]: https://github.com/troglobit/libuev/compare/v1.0.0...v1.0.1
[v1.0.0]: https://github.com/troglobit/libuev/compare/v0.0.1...v1.0.0
[TODO.md]: https://github.com/troglobit/libuev/blob/master/TODO.md
[README.md]: https://github.com/troglobit/libuev/blob/master/README.md
[CHANGLOG.md]: https://github.com/troglobit/libuev/blob/master/CHANGELOG.md
[joystick.c]: https://github.com/troglobit/libuev/blob/master/joystick.c
[Niels Provos]: http://en.wikipedia.org/wiki/Niels_Provos
[MIT license]: http://en.wikipedia.org/wiki/MIT_License
[libev version]: http://libev.schmorp.de/bench.c
[libuevent]: https://code.google.com/p/libuevent/
[Flemming Madsen]: http://www.madsensoft.dk
[Initial announcement]: http://lua-users.org/lists/lua-l/2012-03/msg00510.html

<!--
  -- Local Variables:
  --  mode: markdown
  -- End:
  -->
