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

INCLUDE += -I /usr/include/lua5.1/	
LDFLAGS=-s
CFLAGS=-fPIC -g -Os $(INCLUDE)

MAJOR = 1

all: libuevent.a libuevent.so.$(MAJOR)
lua: luevent.so

libuevent.a: libuevent.o
	$(AR) r $@ $^
	$(CROSS_COMPILE)strip --strip-unneeded $@

libuevent.so.$(MAJOR): libuevent.o
	$(CC) $(LDFLAGS) -shared -o $@ $^ -Wl,-soname=libuevent

luevent.so: luevent.o all
	$(CC) $(LDFLAGS) -shared -o $@ $^ -L . -l uevent -Wl,-soname=luevent

test: all
	$(CC) -g -o test -D NO_DEBUG test.c libuevent.c && ./test

luatest: lua
	lua test.lua

clean:
	rm -f libuevent.o luauevent.o libuevent.so.$(MAJOR) luevent.so test


.PHONY: all test clean luatest lua
