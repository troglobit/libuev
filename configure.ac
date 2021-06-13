AC_INIT(libuev, 2.4.0-beta1, https://github.com/troglobit/libuev/issues)
AC_CONFIG_AUX_DIR(aux)
AM_INIT_AUTOMAKE([1.11 foreign dist-xz])
AM_SILENT_RULES([yes])

AC_CONFIG_SRCDIR(src/uev.c)
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_FILES([Makefile doc/Makefile examples/Makefile src/libuev.pc src/Makefile tests/Makefile])
AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CC
AC_PROG_INSTALL
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
LT_INIT

# Optional features
AC_ARG_ENABLE([examples],
	[AC_HELP_STRING([--enable-examples], [Build libuEv examples/ directory])],
	[], [enable_examples=no])
AM_CONDITIONAL([ENABLE_EXAMPLES], [test "$enable_examples" = yes])

AC_OUTPUT
