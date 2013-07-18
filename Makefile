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
CFLAGS      = -fPIC -g -Os
ARFLAGS     = crus
JUNK        = *~ *.bak *.map .*.d DEADJOE *.gdb *.elf core core.*

OBJS       := libuev.o
SRCS       := $(OBJS:.o=.c)
DEPS       := $(addprefix .,$(SRCS:.c=.d))
VER         = 1
LIBNAME     = libuev
MYLIB       = $(LIBNAME).so
TARGET      = $(MYLIB).$(VER)
STATICLIB   = $(LIBNAME).a

all: $(TARGET)

.c.o:
	@printf "  CC      $@\n"
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

$(TARGET): $(OBJS)
	@printf "  LINK    $@\n"
	@$(CC) $(LDFLAGS) -shared -Wl,-soname=$@ -o $@ $^
	@$(AR) $(ARFLAGS) $(STATICLIB) $^

strip: all
	@printf "  STRIP   %s\n" $(TARGET)
	@$(STRIP) --strip-unneeded $(TARGET) $(STATICLIB)

test: all
	@printf "  TEST    %s\n" $(STATICLIB)
	@$(CC) -g -DNO_DEBUG -o test test.c $(STATICLIB) && ./test

clean:
	-@$(RM) $(JUNK) $(STATICLIB) $(TARGET) *.o test

