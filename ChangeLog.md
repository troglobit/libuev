Change Log
==========

All notable changes to the project are documented in this file.


[v2.4.0][] - 2021-10-03
-----------------------

Please note, this release is a major ABI bump due to changes in `uev_t`
which requires recompiling all programs that use libuEv.

Also, libuEv is built for 64 bit `time_t`, this means all applications
that link with libuEv also must be built with the same build flags.
This applies in particular to systems with GLIBC v2.34, which now
require applications to add the following to their `CPPFLAGS`:

    CPPFLAGS += -D_TIME_BITS=64 -D_FILE_OFFSET_BITS=64

Applications that use `pkg-config` will get these extra flags from the
`libuev.pc` file in the distribution archive.


### Changes
- More examples added for fork + signals + timer
- Add `struct signalfd_siginfo` to `uev_t`, valid for signal watchers.
  Now more metadata is available to signal callbacks

### Fixes
- Fix #24: Replace obsolete autotools macro, by Otto Urpelainen
- Fix #25: GLIBC v2.34 require `-D_FILE_OFFSET_BITS=64` for applications
  and libraries that want a 64-bit `time_t`.  Affects library and test
  programs used for `make check`, as well as all other applications that
  link with libuEv, so the `libuev.pc` file is also updated
- Fix error handling in callbacks in examples


[v2.3.2][] - 2021-02-12
-----------------------

### Changes
- Copyright updates, including LICENSE file, year + author last name
- Add tar.gz archives to distribution
- Enable .sha256 checksums for all tarballs

### Fixes
- Debian packaging fixes only


[v2.3.1][] - 2020-02-22
-----------------------

### Fixes

- Debian packaging fixes only


[v2.3.0][] - 2019-04-06
-----------------------

### Changes

- Support for Linux eventfd, `uev_event_*()` see [API.md][] for details


[v2.2.0][] - 2018-10-04
-----------------------

**NOTE:** You now have to explicitly include `sys/queue.h`, or provide a
          local version of `queue.h`, if your application depends on it.

### Changes
- Replaced BSD `queue.h` doubly-linked lst API with own implementation.
  Making libuEv stand-alone, no longer imposing any particular version
  of `queue.h` on the user
- Enforce `-std=gnu11` to unlock `typeof()` in older GCC versions
- Code cleanup

### Fixes
- Fix missing header deps. in `Makefile.am`, library did not rebuild
  properly if any of the local header files were changed


[v2.1.3][] - 2018-09-06
-----------------------

### Changes
- Update joystick example, use first device, `/dev/input/js0`
- Minor refactor of `uev_exit()`, use `_SAFE` macros to traverse
  list of watchers instead of `while()`
- New API test to verify that `uev_exit()` terminates properly
- Make `uev_signal_stop()` idempotent

### Fixes
- Let `uev_timer_stop()` call `close()` on the timerfd directly, do not
  call `uev_timer_set()` since that may cause lockups or hangs


[v2.1.2][] - 2018-02-27
-----------------------

### Changes
- Minor refactor of event loop after fixing the nasty use-after-free bug
  in v2.1.1.  Code can now be collapsed and noticeably simplified

### Fixes
- Issue #17: Check if `AM_PROG_AR` macro exists before calling it, fixes
  problem building libuEv on systems with older autoconf + automake.
  Patch by Markus Svilans
- Fix minor issue with unit tests, return result of test not event loop


[v2.1.1][] - 2018-01-28
-----------------------

### Fixes
- Fix use after free in main event loop if watcher deletes itself in
  the callback.  I.e., the callback must be the last action for the
  watcher in the event loop.
- Doc timer example fix by @tisyang
- Doc timer updates, non-zero timeout required


[v2.1.0][] - 2017-11-14
-----------------------

### Changes
- Remove event loop error tracking used to trigger a `epoll_create1()`
  at a certain error threshold.  This tracking was first introduced in
  [v1.1.0][], triggered by spurious `EPOLLERR` on I/O watchers
- Unconditionally stop I/O watchers that return `EPOLLERR` or `EPOLLHUP`
  it is up to the watcher callback to clear the error and/or `read()`
  the last few bytes from the descriptor.  HUP usally means EOF, or that
  the remote end of a stream or pipe closed, this may also be signaled
  by `read()` returning zero
- Add missing `--enable-examples` to `configure` script
- Update documentation, both [README.md][] and [API.md][]

### Fixes
- Properly stop and de-register signal and cron/timer watchers from the
  epoll socket in case of errors, problem introduced in [v2.0.0][]
- Mark watcher file descriptor as unintialized on internal error
- Fix double-close of cron/timer watchers.  Problem triggered when the
  timer expires and calls `uev_exit()`, which stops all watchers.  When
  the timer callback returns another call to stop the watcher triggered
  the double `close()`
- Fix unit test's error handling in watcher callbacks, for reference
- Fix use-before-set in cronrun unit test
- Make sure to restart unit test's I/O watchers on `UEV_ERROR`
- Make sure to restart example I/O watchers on `UEV_ERROR`
- Properly check for `UEV_HUP` in unit tests and examples


[v2.0.0][] - 2017-11-11
-----------------------

Beware, this is a major release, introducing incompatible changes to the
failure modes of `uev_run()` and watcher callbacks.  Most users will most
likely not notice any difference, but please read on.

### Changes
- `uev_run()` no longer exits the main event loop if an unrecoverable
  error with a watcher occurs.  Instead, the watcher is disabled and the
  callback is run one last time with `events` set to `UEV_ERROR`
- Watcher callbacks must handle `UEV_ERROR` conditions.  This pertains
  in particular to signal and timer watchers
- Examples and API docs updated with the new failure modes


[v1.6.0][] - 2017-09-18
-----------------------

### Changes
- Support for edge triggered and oneshot event types
- Add `make package` build target to trigger a `.deb` package build
- Support 64 bit `time_t` on 32 bit GLIBC systems

### Fixes
- `bench.c`: Use `signal.h`, not non-standard `sys/signal.h`


[v1.5.2][] - 2016-11-27
-----------------------

### Fixes
- Fix build regression in v1.5.1


[v1.5.1][] - 2016-11-27
-----------------------

### Changes
- Add support for checking if a watcher is active.
- Refactor unit testing framework


[v1.5.0][] - 2016-10-30
-----------------------

### Changes
- Add support for absolute timers with the `uev_cron_*()` API.
- Update build & install instructions in [README.md][]

### Fixes
- Fix `uev_timer_set()` so that it returns error in case the underlying
  Linux timerfd API fails.


[v1.4.2][] - 2016-06-25
-----------------------

### Changes
- Add range checks for `period` and `timeout` to `uev_timer_init()` and
  `uev_timer_set()`.  If either on is `< 0` libuEv now return `ERANGE`.


[v1.4.1][] - 2016-04-22
-----------------------

### Changes
- Add some `pkg-config` integration tips for developers using libuEv
  with GNU autotools in their projects.

### Fixes
- Add missing `SFD_CLOEXEC` and `TFD_CLOEXEC` to signal and timer file
  descriptors.  This prevents these file descriptors from "leaking" into
  sub-processes of the parent thread.  The kernel will atomically close
  these descriptors for forked-off children calling the exec*() family
  of syscalls.
- Minor coding style fixes and simplification of build scripts.


[v1.4.0][] - 2016-03-22
-----------------------

This release changes the header file namespace, which was silently
introduced in [v1.2.1][].  Apologies for any problems this may cause!

### Changes
- Change namespace for header files: `libuev/` to `uev/`, e.g. using
  `pkg-config` your C program must now `#include <uev/uev.h>`
- Support for `EPOLLPRI` events for I/O watchers, thanks to Markus Svilans
- Simplified joystick example
- Updated [README.md][] slightly


[v1.3.1][] - 2016-02-02
-----------------------

### Fixes
- Remove symlinks to Markdown files from GIT
- Distribute and install Markdown files: [README.md][], etc.


[v1.3.0][] - 2016-01-22
-----------------------

### Changes
- Change to GNU configure and build system
- Default optimization is now `-O2`, not `-Os`, which can be a bit
  unpredictable with some cross compiler toolchains.  `-O2` is the
  tried and tested path, and default in autotools
- Added `-Wextra` to the previous `-W -Wall`, drop `-Werror`
  which is supposed to be used by maintainer(s) only.


[v1.2.4][] - 2015-11-23
-----------------------

### Fixes

- Do not allow `VERSION` to be overloaded by build system.
- Make sure we don't inherit `LDFLAGS` from environment.


[v1.2.3][] - 2015-09-17
-----------------------

Very minor release.

### Changes
- [README.md][] updates, mostly cleanup and readability improvements
- `make test` now runs the test case, on behalf of Travis-CI
- Renamed CHANGELOG.md to [ChangeLog.md][]
- Dropped [TODO.md][] from distribution archives, only for devs

### Fixes
- Lots of Markdown syntax fixes in both [README.md][] and ChangeLog
- Silence annoying warning from newer GNU ar in Ubuntu 15.10


[v1.2.2][] - 2015-08-30
-----------------------

Minor bugfix release.

### Changes
- The `ifdef STATIC` in the Makefile has been removed.  Now both the
  .a and .so libraries are built.  The user may then select what to
  install.  This is a change for convenience when using libuEv from
  a GNU Configure & Build based project.

### Fixes
- Fix odd data ordering issue on Debian Jessie in new `uev_private_t`
- Fix issue #6: Segfault when stopping timer before calling `uev_exit()`


[v1.2.1][] - 2015-07-02
-----------------------

### Changes
- Private data members in `uev.h` have now been moved to a new file
  called `private.h`.  This will hopefully make it easier to understand
  what a user of libuEv is allowed to play around with.  Thanks to @vonj
  for the discussions around this!
- All builds of libuEv now default to use `-fPIC`, this bloats the code
  slightly, but ensures that linking works for all use cases, withouth
  introducing unnecessary complexity.

### Fixes
- Fix install/uninstall Makefile recipes so they work for both static
  and dynamic builds.  Also, make sure to install all required headers.
- Jakob Eriksson noticed that `O_CLOEXEC` does not exist in the Debian 6
  EGLIBC, but `EPOLL_CLOEXEC` does, and is also what `epoll_create1()`
  should use.  Thank you @vonj!


[v1.2.0][] - 2015-06-09
-----------------------

API change in event callbacks and fix timers that never start.

### Changes
- Remove first `uev_ctx_t *` argument in callbacks, incompatible API
  change!  Please update all your callbacks if you upgrade.

### Fixes
- Fix timers that accidentally broke in [v1.1.0][].
- Fixes to `bench.c`, it now actually listens to the pipe/socket.


[v1.1.0][] - 2015-03-04
-----------------------

Massively improved error handling.

### Changes
- Handle case when user closes a descriptor *before* stopping a watcher.
- Handle `EPOLLHUP` and `EPOLLERR`.  Restart `epoll(7)` descriptor and
  all watchers when an error count reaches a MAX value -- handles stale
  descriptors or cases when kernel does not notice updated descriptors.
- Return error when stopping a watcher fails.
- Update [README.md][] with new `uev_*_start()` functions.
- Bump dev version to 1.1 due to the number of significant changes.

### Fixes
- Remove `test.c` from `DISTFILES` in `Makefile`.  You need the
  comeplete sources to build the examples now.  Thanks to @karasz for
  the heads up on this and the musl libc issue with missing `queue.h`!
- Fix broken link to [v1.0.5][] in this file.


[v1.0.5][] - 2015-02-15
-----------------------

### Changes
- Add `uev_*_start()` functions.
- Add slightly odd `examples/signal.c` example that utilises `fork()`
  and causes segfault in child.
- Renamed `main.c` to `uev.c`
- Move examples to `examples/` subdirectory
- Simplify automatic dependency calculation
- Add [TODO.md][] for wishlist items
- Add [CHANGELOG.md][ChangeLog.md], to align with http://keepachangelog.com
- Further updates to [README.md][]


[v1.0.4][] - 2015-01-24
-----------------------

Minor documentation and build fixes.

Release mainly targeted for [Finit development](https://github.com/troglobit/finit/).

### Changes
* `test.c` has been simplified/clarified
* Further updates to [README.md][]
* Minor update to Makefile, change how build progress is echoed


[v1.0.3][] - 2015-01-24
-----------------------

This is a very minor release, with a strong focus on documentation. 

### Changes
- [README.md][], massively updated
- API documentation, updated
- Travis CI integration added, https://travis-ci.org/troglobit/libuev
- Coverity Scan integration, https://scan.coverity.com/projects/3846
- Clang scan-build support added


[v1.0.2][] - 2015-01-07
-----------------------

### Fixes
- Fix broken `make dist` target in [v1.0.1][].


[v1.0.1][] - 2015-01-07
-----------------------

### Changes
- Cleanup and rewrite of [README.md][] after first audit by @vonj
- Added API and example section to [README.md][]
- New [joystick.c][] example added
- `main.c:uev_run()`: Document mysterious flags parameter


[v1.0.0][] - 2013-08-06
-----------------------

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
- Make sure to state the [MIT license][] correctly in all files
- Import [Niels Provos][]' `bench.c`, the [libev version][]


v0.0.1 - 2012-03-17
-------------------

[Initial announcement][] of [libuevent][] by [Flemming Madsen][] to the
Lua users mailing list.


[UNRELEASED]: https://github.com/troglobit/libuev/compare/v2.4.0...HEAD
[v2.4.0]: https://github.com/troglobit/libuev/compare/v2.3.2...v2.4.0
[v2.3.2]: https://github.com/troglobit/libuev/compare/v2.3.1...v2.3.2
[v2.3.1]: https://github.com/troglobit/libuev/compare/v2.3.0...v2.3.1
[v2.3.0]: https://github.com/troglobit/libuev/compare/v2.2.0...v2.3.0
[v2.2.0]: https://github.com/troglobit/libuev/compare/v2.1.3...v2.2.0
[v2.1.3]: https://github.com/troglobit/libuev/compare/v2.1.2...v2.1.3
[v2.1.2]: https://github.com/troglobit/libuev/compare/v2.1.1...v2.1.2
[v2.1.1]: https://github.com/troglobit/libuev/compare/v2.1.0...v2.1.1
[v2.1.0]: https://github.com/troglobit/libuev/compare/v2.0.0...v2.1.0
[v2.0.0]: https://github.com/troglobit/libuev/compare/v1.6.0...v2.0.0
[v1.6.0]: https://github.com/troglobit/libuev/compare/v1.5.2...v1.6.0
[v1.5.2]: https://github.com/troglobit/libuev/compare/v1.5.1...v1.5.2
[v1.5.1]: https://github.com/troglobit/libuev/compare/v1.5.0...v1.5.1
[v1.5.0]: https://github.com/troglobit/libuev/compare/v1.4.2...v1.5.0
[v1.4.2]: https://github.com/troglobit/libuev/compare/v1.4.1...v1.4.2
[v1.4.1]: https://github.com/troglobit/libuev/compare/v1.4.0...v1.4.1
[v1.4.0]: https://github.com/troglobit/libuev/compare/v1.3.1...v1.4.0
[v1.3.1]: https://github.com/troglobit/libuev/compare/v1.3.0...v1.3.1
[v1.3.0]: https://github.com/troglobit/libuev/compare/v1.2.3...v1.3.0
[v1.2.3]: https://github.com/troglobit/libuev/compare/v1.2.2...v1.2.3
[v1.2.2]: https://github.com/troglobit/libuev/compare/v1.2.1...v1.2.2
[v1.2.1]: https://github.com/troglobit/libuev/compare/v1.2.0...v1.2.1
[v1.2.0]: https://github.com/troglobit/libuev/compare/v1.1.0...v1.2.0
[v1.1.0]: https://github.com/troglobit/libuev/compare/v1.0.5...v1.1.0
[v1.0.5]: https://github.com/troglobit/libuev/compare/v1.0.4...v1.0.5
[v1.0.4]: https://github.com/troglobit/libuev/compare/v1.0.3...v1.0.4
[v1.0.3]: https://github.com/troglobit/libuev/compare/v1.0.2...v1.0.3
[v1.0.2]: https://github.com/troglobit/libuev/compare/v1.0.1...v1.0.2
[v1.0.1]: https://github.com/troglobit/libuev/compare/v1.0.0...v1.0.1
[v1.0.0]: https://github.com/troglobit/libuev/compare/v0.0.1...v1.0.0
[TODO.md]: https://github.com/troglobit/libuev/blob/master/TODO.md
[API.md]: https://github.com/troglobit/libuev/blob/master/API.md
[README.md]: https://github.com/troglobit/libuev/blob/master/README.md
[ChangeLog.md]: https://github.com/troglobit/libuev/blob/master/ChangeLog.md
[joystick.c]: https://github.com/troglobit/libuev/blob/master/joystick.c
[Niels Provos]: http://en.wikipedia.org/wiki/Niels_Provos
[MIT license]: http://en.wikipedia.org/wiki/MIT_License
[libev version]: http://libev.schmorp.de/bench.c
[libuevent]: https://code.google.com/p/libuevent/
[Flemming Madsen]: http://www.madsensoft.dk
[Initial announcement]: http://lua-users.org/lists/lua-l/2012-03/msg00510.html
