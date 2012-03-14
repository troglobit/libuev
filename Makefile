##
# @file Makefile
# Make the micro eventlib, lua bindings and tests
#
# $Id$
#
# (C) Copyright 2012 flemming.madsen at madsensoft.dk. See libuevent.h
##

CROSS_COMPILE=
#CROSS_COMPILE=arm-linux-uclibc-
CC=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar

LDFLAGS=-s
CFLAGS=-fPIC -g -Os

MAJOR = 1

all: libuevent.a libuevent.so.$(MAJOR)
lua: luauevent.so

libuevent.a: libuevent.o
	$(AR) r $@ $^
	$(CROSS_COMPILE)strip --strip-unneeded $@

libuevent.so.$(MAJOR): libuevent.o
	$(CC) $(LDFLAGS) -shared -o $@ $^ -Wl,-soname=libuevent

luauevent.so: luauevent.o all
	$(CC) $(LDFLAGS) -shared -o $@ $^ -L . -l uevent -Wl,-soname=uevent

test: all
	$(CC) -g -o test -D NO_DEBUG test.c libuevent.c && ./test

luatest: lua
	lua test.lua

clean:
	rm -f libuevent.o luauevent.o libuevent.so.$(MAJOR) luauevent.so test


.PHONY: all test clean luatest lua
