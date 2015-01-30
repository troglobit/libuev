# -*-Makefile-*- for libuEv
#
# Copyright (c) 2012       Flemming Madsen <flemming!madsen()madsensoft!dk>
# Copyright (c) 2013-2015  Joachim Nilsson <troglobit()gmail!com>
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

#VERSION    = $(shell git tag -l | tail -1)
VERSION    ?= 1.0.4
NAME        = libuev
PKG         = $(NAME)-$(VERSION)
ARCHIVE     = $(PKG).tar.xz

CC         ?= $(CROSS)gcc
AR         ?= $(CROSS)ar
STRIP      ?= $(CROSS)strip
CFLAGS     += -fPIC -Os
CPPFLAGS   += -W -Wall -Werror
ARFLAGS     = crus
JUNK        = *~ *.bak *.map .*.d DEADJOE *.gdb *.elf core core.*

ROOTDIR    ?= $(shell pwd)
LIBNAME     = $(NAME)
prefix     ?= /usr/local
libdir     ?= $(prefix)/lib
datadir    ?= $(prefix)/share/doc/$(LIBNAME)
incdir     ?= $(prefix)/include
DISTFILES   = README LICENSE test.c
HEADER      = uev.h

OBJS       := main.o io.o timer.o signal.o
SRCS       := $(OBJS:.o=.c)
DEPS       := $(addprefix .,$(SRCS:.c=.d))
VER         = 1
SOLIB       = $(LIBNAME).so.$(VER)
SYMLIB      = $(LIBNAME).so
STATICLIB   = $(LIBNAME).a
TARGET      = $(STATICLIB) $(SOLIB)

# Pattern rules
.c.o:
	@printf "  CC      $(subst $(ROOTDIR)/,,$(shell pwd)/)$@\n"
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
	@install -m 0644 $(HEADER) $(DESTDIR)$(incdir)/$(HEADER)
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
	@size $(TARGET)

test: Makefile test.o $(STATICLIB)
	@printf "  TEST    %s\n" $(STATICLIB)
	@$(CC) $(CPPFLAGS) -g -o test test.c $(STATICLIB) && ./test

bench: Makefile bench.o $(STATICLIB)
	@printf "  BENCH   %s\n" $(STATICLIB)
	@$(CC) $(CPPFLAGS) -g -o bench bench.c $(STATICLIB) && ./bench

joystick: Makefile joystick.o $(STATICLIB)
	@printf "  JOTST   %s\n" $(STATICLIB)
	@$(CC) $(CPPFLAGS) -g -o joystick joystick.c $(STATICLIB) && ./joystick

# Runs Clang scan-build on the whole tree
check: clean
	@scan-build $(MAKE) all

clean:
	-@$(RM) $(TARGET) *.o test joystick bench

distclean: clean
	-@$(RM) $(DEPS) $(JUNK)

dist:
	@echo "Building .xz tarball of $(PKG) in parent dir..."
	git archive --format=tar --prefix=$(PKG)/ v$(VERSION) | xz >../$(ARCHIVE)
	@(cd ..; md5sum $(ARCHIVE) | tee $(ARCHIVE).md5)

# Include automatically generated rules
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
-include $(DEPS)
endif
endif

