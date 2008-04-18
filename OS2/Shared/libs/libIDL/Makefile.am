# -*- mode: makefile -*-
#
# libIDL Makefile.am
#
SUBDIRS = include
INCLUDES =                             \
	-DYYDEBUG=1                    \
	-DYYERROR_VERBOSE=1            \
	$(LIBIDL_CFLAGS)               \
	-I$(top_builddir)/include      \
	-DG_LOG_DOMAIN=\"libIDL\"      \
	$(WARN_CFLAGS)

YFLAGS = -d -v 2>/dev/null

lexer.c: $(srcdir)/lexer.l parser.h
	@rm -f $@
	$(LEX) $(LFLAGS) -t $(srcdir)/lexer.l | sed -e 's/yy/__IDL_/g' >$@

stamp-parser: $(srcdir)/parser.y
	$(YACC) $(YFLAGS) $(srcdir)/parser.y
	touch $@

parser.c: stamp-parser
	test -f y.tab.c && mv -f y.tab.c parser.c || touch $@

parser.h: stamp-parser
	test -f y.tab.h && mv -f y.tab.h parser.h || touch $@

MOSTLYCLEANFILES	= parser.output y.output

EXTRA_DIST		= BUGS parser.y lexer.l \
			  Makefile.msc README.win32 libIDL-zip.in \
			  libIDL.def \
			  HACKING MAINTAINERS

info_TEXINFOS		= libIDL2.texi
lib_LTLIBRARIES		= libIDL-2.la
noinst_PROGRAMS		= tstidl

bin_SCRIPTS		= libIDL-config-2

BUILT_SOURCES		= lexer.c parser.c parser.h stamp-parser

libIDL_2_la_SOURCES	= parser.c parser.h lexer.c \
			  ns.c util.c util.h rename.h
libIDL_2_la_LDFLAGS	= -version-info $(LT_VERSION) $(LIBIDL_LIBS) -no-undefined

tstidl_SOURCES		= tstidl.c
tstidl_LDADD		= libIDL-2.la $(LIBIDL_LIBS)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA= libIDL-2.0.pc

CLEANFILES = $(BUILT_SOURCES)

dist-hook:
	cd $(distdir) ; rm -f $(CLEANFILES)
