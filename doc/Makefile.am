dist_doc_DATA = API.md
EXTRA_DIST    = Doxyfile.in

if ENABLE_HTML
pkghtmldir     = $(docdir)/html
pkghtml_DATA   = html/*

#
# Doxygen rules from m4/ax_prog_doxygen.m4
#
@DX_RULES@

$(pkghtml_DATA): doxygen-doc

all-local: doc

clean-local:
	-rm -rf html

doc:  doxygen-doc
endif
