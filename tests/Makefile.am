EXTRA_DIST      = check.h
CLEANFILES      = *~ *.trs *.log

TESTS           =
TESTS          += active
TESTS          += api
TESTS          += complete
TESTS          += cronrun
TESTS          += signal
TESTS          += timer

check_PROGRAMS  = $(TESTS)

# Ignore warnings about unused result, in e.g. write()
CFLAGS          = -W -Wall -Wextra -Wno-unused-result -Wno-unused-parameter
CPPFLAGS        = -D_GNU_SOURCE
LDADD           = -L../src ../src/libuev.la
