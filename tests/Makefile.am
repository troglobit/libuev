EXTRA_DIST      = check.h
CLEANFILES      = *~ *.trs *.log

TESTS           =
TESTS          += timer

check_PROGRAMS  = $(TESTS)

# Ignore warnings about unused result, in e.g. write()
CFLAGS          = -W -Wall -Wextra -Wno-unused-result
CPPFLAGS        = -D_GNU_SOURCE
LDADD           = -L../ ../libuev.la
