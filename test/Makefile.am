EXTRA_DIST      = check.h
CLEANFILES      = *~ *.trs *.log

TESTS           =
TESTS          += active
TESTS          += api
TESTS          += complete
TESTS          += cronrun
TESTS          += signal
TESTS          += timer
TESTS          += event

check_PROGRAMS  = $(TESTS)

CPPFLAGS        = -D_GNU_SOURCE -D_TIME_BITS=64 -D_FILE_OFFSET_BITS=64
LDADD           = -L../src ../src/libuev.la
