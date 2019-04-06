lib_LTLIBRARIES     = libuev.la
libuev_la_SOURCES   = uev.c uev.h private.h io.c timer.c signal.c cron.c event.c
libuev_la_CPPFLAGS  = -D_GNU_SOURCE -D_TIME_BITS=64
libuev_la_CFLAGS    = -W -Wall -Wextra -std=gnu11
libuev_la_LDFLAGS   = $(AM_LDFLAGS) -version-info 4:0:2

noinst_PROGRAMS     = bench
bench_CPPFLAGS      = -D_GNU_SOURCE
bench_LDADD         = libuev.la

pkgconfigdir        = $(libdir)/pkgconfig
pkgincludedir       = $(includedir)/uev
pkgconfig_DATA      = libuev.pc
pkginclude_HEADERS  = uev.h private.h
