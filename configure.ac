AC_INIT(libuev, 2.4.1-dev, https://github.com/troglobit/libuev/issues)
AC_CONFIG_AUX_DIR(aux)
AM_INIT_AUTOMAKE([1.11 foreign dist-xz])
AM_SILENT_RULES([yes])

AC_CONFIG_SRCDIR(src/uev.c)
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_FILES([Makefile doc/Makefile doc/Doxyfile examples/Makefile
			  src/libuev.pc src/Makefile test/Makefile])
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

# Check for Doxygen and enable its features.
# For details, see m4/ax_prog_doxygen.m4 and
# http://www.bioinf.uni-freiburg.de/~mmann/HowTo/automake.html#doxygenSupport
DX_DOXYGEN_FEATURE(ON)
DX_DOT_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)
DX_CHM_FEATURE(OFF)
DX_HTML_FEATURE(ON)
DX_MAN_FEATURE(OFF)
DX_INIT_DOXYGEN(${PACKAGE_NAME}, [${top_builddir}/doc/Doxyfile], [${top_builddir}/doc])
AM_CONDITIONAL(ENABLE_DOXYGEN,[test "x${DX_FLAG_doc}" = x1])
AM_CONDITIONAL(ENABLE_HTML,[test "x${DX_FLAG_html}" = x1])

AC_OUTPUT
