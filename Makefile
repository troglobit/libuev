# -*-Makefile-*- for libuev
#
# Copyright (c) 2012  Flemming Madsen <flemming!madsen()madsensoft!dk>
# Copyright (c) 2013  Joachim Nilsson <troglobit()gmail!com>
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
.PHONY: all test clean

CC          = $(CROSS_COMPILE)gcc
AR          = $(CROSS_COMPILE)ar
STRIP       = $(CROSS_COMPILE)strip
CFLAGS     += -fPIC -Os
CPPFLAGS   += -W -Wall -Iinclude
ARFLAGS     = crus
JUNK        = *~ *.bak *.map .*.d DEADJOE *.gdb *.elf core core.*

LIBNAME     = libuev
prefix     ?= /usr/local
libdir     ?= $(prefix)/lib
datadir    ?= $(prefix)/share/doc/$(LIBNAME)
incdir     ?= $(prefix)/include/$(LIBNAME)
DISTFILES   = README LICENSE test.c
INCLUDES    = uev.h queue.h

OBJS       := libuev.o
SRCS       := $(OBJS:.o=.c)
DEPS       := $(addprefix .,$(SRCS:.c=.d))
VER         = 1
SOLIB       = $(LIBNAME).so.$(VER)
SYMLIB      = $(LIBNAME).so
STATICLIB   = $(LIBNAME).a
TARGET      = $(SOLIB) $(STATICLIB)

# Pattern rules
.c.o:
	@printf "  CC      $@\n"
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

# Smart autodependecy generation via GCC -M.
.%.d: %.c
	@$(SHELL) -ec "$(CC) -MM $(CFLAGS) $(CPPFLAGS) $< 2>/dev/null \
		| sed 's,.*: ,$*.o $@ : ,g' > $@; \
                [ -s $@ ] || rm -f $@"

# Build rules
all: $(TARGET)

$(SOLIB): Makefile $(OBJS)
	@printf "  LINK    $@\n"
	@$(CC) $(LDFLAGS) -shared -Wl,-soname=$@ -o $@ $(OBJS)

$(STATICLIB): Makefile $(OBJS)
	@printf "  ARCHIVE $@\n"
	@$(AR) $(ARFLAGS) $@ $(OBJS)

install: strip
	@install -d $(DESTDIR)$(libdir)
	@install $(SOLIB) $(DESTDIR)$(prefix)/lib/$(SOLIB)
	@install $(STATICLIB) $(DESTDIR)$(prefix)/lib/$(STATICLIB)
	@ln -sf $(SOLIB) $(DESTDIR)$(prefix)/lib/$(SYMLIB)
	@install -d $(DESTDIR)$(incdir)
	@for file in $(INCLUDES); do \
		install -m 0644 include/libuev/$$file $(DESTDIR)$(incdir)/$$file; \
	done
	@install -d $(DESTDIR)$(datadir)
	@for file in $(DISTFILES); do \
		install -m 0644 $$file $(DESTDIR)$(datadir)/$$file; \
	done

uninstall:
	-@$(RM) -r $(DESTDIR)$(datadir)
	-@$(RM) -r $(DESTDIR)$(incdir)
	-@$(RM) $(DESTDIR)$(prefix)/lib/$(LIBNAME)*

strip: $(TARGET)
	@printf "  STRIP   %s\n" $(TARGET)
	@$(STRIP) --strip-unneeded $(TARGET)

test: Makefile test.o $(STATICLIB)
	@printf "  TEST    %s\n" $(STATICLIB)
	@$(CC) $(CPPFLAGS) -g -o test test.c $(STATICLIB) && ./test

clean:
	-@$(RM) $(TARGET) *.o test

distclean: clean
	-@$(RM) $(DEPS) $(JUNK)

# Include automatically generated rules
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
-include $(DEPS)
endif
endif

