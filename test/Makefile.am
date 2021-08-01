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

CPPFLAGS        = -D_GNU_SOURCE
LDADD           = -L../src ../src/libuev.la
