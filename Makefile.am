ACLOCAL_AMFLAGS     = -I m4

lib_LTLIBRARIES     = libuev.la
pkgincludedir       = $(includedir)/uev
pkginclude_HEADERS  = uev.h private.h queue.h
libuev_la_SOURCES   = uev.c io.c timer.c signal.c cron.c

check_PROGRAMS      = bench
bench_CPPFLAGS      = -D_GNU_SOURCE
bench_LDADD         = $(top_srcdir)/libuev.la

libuev_la_CPPFLAGS  = -D_GNU_SOURCE
libuev_la_CFLAGS    = -W -Wall -Wextra
libuev_la_LDFLAGS   = $(AM_LDFLAGS) -version-info 1:1:0

pkgconfigdir        = $(libdir)/pkgconfig
pkgconfig_DATA      = libuev.pc
doc_DATA            = README.md LICENSE
EXTRA_DIST          = README.md LICENSE AUTHORS ChangeLog.md

DIST_SUBDIRS        = examples
SUBDIRS             = .
if ENABLE_EXAMPLES
  SUBDIRS          += examples
endif

test joystick: all-recursive
	./examples/$@

check-local:
	@echo "  BENCH    $(PACKAGE)"
	@./bench
if ENABLE_EXAMPLES
	@echo "  TEST     $(PACKAGE)"
	@./examples/test
endif

## Generate MD5 checksum file
MD5 = md5sum
md5-dist:
	@for file in $(DIST_ARCHIVES); do	\
		$(MD5) $$file > $$file.md5;	\
	done

## Check if tagged in git
release-hook:
	if [ ! `git tag | grep $(PACKAGE_VERSION)` ]; then				\
		echo;									\
		printf "\e[1m\e[41mCannot find release tag $(PACKAGE_VERSION)\e[0m\n";	\
		printf "\e[1m\e[5mDo release anyway?\e[0m "; read yorn;			\
		if [ "$$yorn" != "y" -a "$$yorn" != "Y" ]; then				\
			printf "OK, aborting release.\n";				\
			exit 1;								\
		fi;									\
		echo;									\
	else										\
		echo;									\
		printf "\e[1m\e[42mFound GIT release tag $(PACKAGE_VERSION)\e[0m\n";	\
		printf "\e[1m\e[44m>>Remember to push tags!\e[0m\n";			\
		echo;									\
	fi

## Target to run when building a release
release: dist release-hook md5-dist
	@for file in $(DIST_ARCHIVES); do						\
		printf "$$file    \tDistribution tarball\n";				\
		printf "$$file.md5\t"; cat $$file.md5 | cut -f1 -d' ';			\
		mv $$file* ../;								\
	done
