AC_INIT(libuEv, 1.4.0, https://github.com/troglobit/libuev/issues, libuev)
AM_INIT_AUTOMAKE([1.11 foreign no-dist-gzip dist-xz])
AM_SILENT_RULES([yes])

AC_CONFIG_SRCDIR(uev.c)
AM_CONFIG_HEADER(config.h)
AC_CONFIG_FILES([Makefile libuev.pc examples/Makefile])
AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CC
AM_PROG_AR
LT_INIT

AC_ARG_ENABLE([examples],
	[AS_HELP_STRING([--enable-examples], [build examples])],
	[enable_examples=yes], [])
AM_CONDITIONAL([ENABLE_EXAMPLES], [test "$enable_examples" = yes])

AC_HEADER_STDC
AC_C_CONST

AC_OUTPUT
