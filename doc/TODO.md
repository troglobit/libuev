LibuEv TODO
===========

* Add standard UNIX backend with `select()` and `timer_create()`
  See SMCRoute and toolbox for examples
* Port to *BSD kqueue API, <http://en.wikipedia.org/wiki/Kqueue>
  Also in UNIX Network Progamming, by W. Richard Stevens 3rd ed.
  More porting ideas and help, see the following GitHub issue;
  <https://github.com/troglobit/libuev/issues/20>
* Restore Lua bindings now that libuEv itself has stabilized

