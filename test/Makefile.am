EXTRA_DIST      = check.h
CLEANFILES      = *~ *.trs *.log
AM_CFLAGS       = -W -Wall -Wextra -Wno-unused-result -Wno-unused-parameter
AM_CPPFLAGS     = -D_GNU_SOURCE -D_TIME_BITS=64 -D_FILE_OFFSET_BITS=64
AM_LDFLAGS      = -L../src ../src/libuev.la

TESTS           =
TESTS          += active
TESTS          += api
TESTS          += complete
TESTS          += cronrun
TESTS          += signal
TESTS          += timer
TESTS          += event

check_PROGRAMS  = $(TESTS)
