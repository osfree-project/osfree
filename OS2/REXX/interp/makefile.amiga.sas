#
# Makefile for Regina REXX Interpreter
#
# Using SAS/C on Amiga, dedicated to Regina Henke
#
SRCDIR	    = /Regina-3.1
DEMODIR	    = $(SRCDIR)/demo
CC	    = sc
CEXTRA	    = ANSI DEF _POSIX_SOURCE DEF NO_EXTERNAL_QUEUES
EEXTRA	    =
DYN_COMP    = -fPIC -DDYNAMIC
LIBS	    = MATH=s
SHLIBS	    = $(LIBS)
SHL	    = so
LD_RXLIB1   = ld -rpath . -rpath /usr/local/lib -shared -o $@
LD_RXLIB2   = -L. -lregina
DYNAMIC_LDFLAGS = netinclude:/netlib/net.lib #-Wl,-rpath . -Wl,-rpath /usr/local/lib
SHL_LD	    = $(CC) -o $(LIBPRE)$(LIBFILE).$(SHL).$(VERDOT) -shared -Wl,-soname,lib$(LIBFILE).$(SHL).$(ABI),-stats $(SHOFILES)
SHL_TARGETS = $(LIBPRE)$(LIBFILE).a $(LIBPRE)$(LIBFILE).$(SHL) rxtest1.rxlib rxtest2.rxlib
LDEXTRA	    =
LIBPRE	    =
LIBFILE	    = regina
#OBJECTS     = $(OFILES)
OSAVE	    = .o.save
MV	    = -rename
#
MT_FILE = mt_notmt
SHLFILE = regina
OBJ = o
EXE =
LIBPST = lib
LIBEXE = oml
LIBFLAGS = $(LIBPRE)$(LIBFILE).$(LIBPST)
LIBLINK = $(LIBFILE).$(LIBPST)
LINKSHL = $(LIBPRE)$(SHLFILE).$(SHL)
RXLIB = rxlib
SHL1 = rm -f $(LIBPRE)$(SHLFILE).$(SHL).$(ABI)
SHL2 = ln -s $(LIBPRE)$(SHLFILE).$(SHL).$(VERDOT) $(LIBPRE)$(LIBFILE).$(SHL).$(ABI)
SHL3 = rm -f $(LIBPRE)$(SHLFILE).$(SHL)
SHL4 = ln -s $(LIBPRE)$(SHLFILE).$(SHL).$(ABI) $(LIBPRE)$(LIBFILE).$(SHL)

RANLIB_LIB  = #ranlib $(LIBPRE)$(LIBFILE).a

#
# If your compiler can handle the combination of: -c -o file.sho, then
# uncomment the macro CC2O and comment out the 3 macros before CC2O
#
O2SHO=#-mv `basename $@ .sho`.o $@
O2SAVE=#-mv `basename $@ .sho`.o `basename $@ .sho`.o.save
SAVE2O=#-mv `basename $@ .sho`.o.save `basename $@ .sho`.o
CC2O=#-o $@
#
# Include the common rules for the interpreter
#
#
# Some settings that you may wish to change, these are used only
# for the 'install' target:
#
#   TOPDIR    The top of the directory hierarchy in which to install
#   BINDIR    Where to install the compiled 'rexx' program
#   LIBDIR    Where to install the 'regina' library
#   MANEXT    The extention of the manual page
#   MANDIR    Where to install the manual page
#   PROGNAME  What to call the compiled program
#
TOPDIR = /usr/local
BINDIR = $(TOPDIR)/bin
LIBDIR = $(TOPDIR)/lib
MANEXT = l
MANDIR = $(TOPDIR)/man/man$(MANEXT)
ABI=1.0
VER=22
VERDOT=2.2

MISCDEFS = IDIR $(SRCDIR) #-DR2PERL
#DEBUG	  = #-g -DNDEBUG #-Dlint
PROF	 = #-p #g
OPTIMIZE = IGN=315,304 PARM=r DEF NDEBUG OPT #CPU=68020

COPT = $(DEBUGFLAG) $(CEXTRA) $(OPTIMIZE) $(PROF) $(MISCDEFS)
LINKOPT = LINK $(DEBUGFLAG) $(OPTIMIZE) $(PROF) $(EEXTRA)
OTHERCOPT = $(DEBUGFLAG) $(OPTIMIZE) $(PROF) $(CEXTRA) $(MISCDEFS)

YACC = yacc
#YACC = bison -ly #-d #-lyt

LEX = lex
#LEX = flex -8 #-d
#LL = -L/store/lib -lfl

README = $(SRCDIR)/README.* COPYING-LIB

FILES = funcs builtin error variable interprt debug dbgfuncs \
	memory parsing files misc unxfuncs arxfuncs os2funcs cmsfuncs shell rexxext stack \
	tracing interp cmath convert strings library strmath signals \
	macros envir wrappers options os_unx alloca mt_notmt nosaa instore #r2perl

CSRCFILES = $(SRCDIR)/funcs.c $(SRCDIR)/builtin.c $(SRCDIR)/error.c $(SRCDIR)/variable.c \
	$(SRCDIR)/interprt.c $(SRCDIR)/debug.c $(SRCDIR)/dbgfuncs.c $(SRCDIR)/memory.c $(SRCDIR)/parsing.c $(SRCDIR)/files.c \
	$(SRCDIR)/misc.c $(SRCDIR)/unxfuncs.c $(SRCDIR)/arxfuncs.c $(SRCDIR)/os2funcs.c $(SRCDIR)/cmsfuncs.c $(SRCDIR)/shell.c $(SRCDIR)/rexxext.c $(SRCDIR)/stack.c \
	$(SRCDIR)/tracing.c $(SRCDIR)/interp.c $(SRCDIR)/cmath.c $(SRCDIR)/convert.c $(SRCDIR)/strings.c $(SRCDIR)/library.c \
	$(SRCDIR)/strmath.c $(SRCDIR)/signals.c $(SRCDIR)/macros.c $(SRCDIR)/envir.c $(SRCDIR)/expr.c \
	$(SRCDIR)/wrappers.c $(SRCDIR)/options.c $(SRCDIR)/os_unx.c $(SRCDIR)/alloca.c $(SRCDIR)/mt_notmt $(SRCDIR)/nosaa $(SRCDIR)/instore.c #$(SRCDIR)/r2perl.c

OFILES = funcs.$(OBJ) builtin.$(OBJ) error.$(OBJ) variable.$(OBJ) \
	interprt.$(OBJ) debug.$(OBJ) dbgfuncs.$(OBJ) memory.$(OBJ) parsing.$(OBJ) files.$(OBJ) \
	misc.$(OBJ) unxfuncs.$(OBJ) arxfuncs.$(OBJ) cmsfuncs.$(OBJ) os2funcs.$(OBJ) shell.$(OBJ) rexxext.$(OBJ) stack.$(OBJ) \
	tracing.$(OBJ) interp.$(OBJ) cmath.$(OBJ) convert.$(OBJ) strings.$(OBJ) library.$(OBJ) \
	strmath.$(OBJ) signals.$(OBJ) macros.$(OBJ) envir.$(OBJ) expr.$(OBJ) alloca.$(OBJ) \
	yaccsrc.$(OBJ) lexsrc.$(OBJ) options.$(OBJ) os_unx.$(OBJ) wrappers.$(OBJ) $(MT_FILE).$(OBJ) instore.$(OBJ) #nosaa.$(OBJ) r2perl.$(OBJ)
OBJECTS	    = $(OFILES)

SHOFILES = funcs.sho builtin.sho error.sho variable.sho \
	interprt.sho debug.sho dbgfuncs.sho memory.sho parsing.sho files.sho \
	misc.sho unxfuncs.sho arxfuncs.sho cmsfuncs.sho shell.sho os2funcs.sho rexxext.sho stack.sho \
	tracing.sho interp.sho cmath.sho convert.sho strings.sho library.sho \
	strmath.sho signals.sho macros.sho envir.sho expr.sho alloca.sho \
	yaccsrc.sho lexsrc.sho wrappers.sho options.sho os_unx.sho \
	drexx.sho client.sho rexxsaa.sho

DEMODIR = $(SRCDIR)/demo
DEMOFILES = $(DEMODIR)/*
TRIPDIR = $(SRCDIR)/trip
TRIPFILES = $(TRIPDIR)/*

ALLSRC = $(CFILES) $(HFILES)

HFILES = $(SRCDIR)/rexx.h $(SRCDIR)/defs.h $(SRCDIR)/extern.h $(SRCDIR)/regina_t.h \
		$(SRCDIR)/config.h $(SRCDIR)/strings.h $(SRCDIR)/configur.h $(SRCDIR)/yaccsrc.h \
		$(SRCDIR)/rxiface.h $(SRCDIR)/rexxsaa.h $(SRCDIR)/utsname.h
CFILES = $(SRCDIR)/yaccsrc.c $(SRCDIR)/lexsrc.c $(CSRCFILES) $(SRCDIR)/execiser.c \
		$(SRCDIR)/rexxsaa.c $(SRCDIR)/rexx.c $(SRCDIR)/regina.c $(SRCDIR)/client.c \
		$(SRCDIR)/r2perl.c $(SRCDIR)/test1.c $(SRCDIR)/test2.c

JUNK = $(OFILES) rexx.o client.o drexx.o rexxsaa.o execiser.o \#?.lnk

#.SUFFIXES: .sho $(SUFFIXES)
#
# Do not use default suffixes rule
#
.SUFFIXES:

all : rexx$(EXE) execiser$(EXE) rxstack$(EXE) rxqueue$(EXE) #$(SHL_TARGETS)

$(SRCDIR)/rexx.h : $(SRCDIR)/extern.h $(SRCDIR)/strings.h $(SRCDIR)/defs.h $(SRCDIR)/regina_t.h $(SRCDIR)/mt.h

$(SRCDIR)/mt.h : $(SRCDIR)/$(MT_FILE).h

rexx$(EXE) : $(OFILES) rexx.$(OBJ)
	$(PURIFY) $(CC) $(LINKOPT) $(STATIC_LDFLAGS) TO rexx $(OBJECTS) rexx.$(OBJ) $(LIBS)
	$(LDEXTRA)

regina$(EXE) : $(LIBPRE)$(SHLFILE).$(SHL) regina.$(OBJ)
	$(PURIFY) $(CC) $(LINKOPT) $(DYNAMIC_LDFLAGS) -o regina$(EXE) regina.$(OBJ) $(LINKSHL) $(LIBS) $(SHLIBS)
	$(LDEXTRA)

execiser$(EXE) : execiser.$(OBJ) $(LIBPRE)$(LIBFILE).$(LIBPST)
	$(PURIFY) $(CC) $(LINKOPT) execiser.$(OBJ) $(LIBLINK) $(SHLIBS)
	$(LDEXTRA)

threader$(EXE) : threader.$(OBJ) $(EXECISER_DEP)
	$(PURIFY) $(CC) $(LINKOPT) -o threader threader.$(OBJ) $(LIBLINK) $(BOTHLIBS) $(SHLIBS)
	$(LDEXTRA)

rxstack$(EXE) : rxstack.$(OBJ) erexxbif.$(OBJ) eextstack.$(OBJ)
	$(PURIFY) $(CC) $(LINKOPT) $(DYNAMIC_LDFLAGS) TO rxstack$(EXE) rxstack.$(OBJ) erexxbif.$(OBJ) eextstack.$(OBJ) $(GETOPT) $(BOTHLIBS) $(SHLIBS)
	$(LDEXTRA)

rxqueue$(EXE) : rxqueue.$(OBJ) erexxbif.$(OBJ) eextstack.$(OBJ)
	$(PURIFY) $(CC) $(LINKOPT) $(DYNAMIC_LDFLAGS) TO rxqueue$(EXE) rxqueue.$(OBJ) erexxbif.$(OBJ) eextstack.$(OBJ) $(GETOPT) $(BOTHLIBS) $(SHLIBS)
	$(LDEXTRA)

$(LIBPRE)$(LIBFILE).$(LIBPST): $(OFILES) client.$(OBJ) drexx.$(OBJ) rexxsaa.$(OBJ)
	$(LIBEXE) $(LIBFLAGS) $(OBJECTS) client.$(OBJ) drexx.$(OBJ) rexxsaa.$(OBJ)
	$(RANLIB_LIB)

$(LIBPRE)$(SHLFILE).$(SHL): $(SHOFILES)
	$(SHL_LD)
	$(SHL1)
	$(SHL2)
	$(SHL3)
	$(SHL4)

#
# Rules for static objects
#
funcs.$(OBJ) :	 $(SRCDIR)/funcs.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/funcs.c

builtin.$(OBJ) : $(SRCDIR)/builtin.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/builtin.c

error.$(OBJ) :	 $(SRCDIR)/error.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/error.c

variable.$(OBJ) :$(SRCDIR)/variable.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/variable.c

interprt.$(OBJ) :$(SRCDIR)/interprt.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/interprt.c

debug.$(OBJ) :	 $(SRCDIR)/debug.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/debug.c

dbgfuncs.$(OBJ) :$(SRCDIR)/dbgfuncs.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/dbgfuncs.c

memory.$(OBJ) :	 $(SRCDIR)/memory.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/memory.c

parsing.$(OBJ) : $(SRCDIR)/parsing.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/parsing.c

files.$(OBJ) :	 $(SRCDIR)/files.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/files.c

misc.$(OBJ) :	 $(SRCDIR)/misc.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/misc.c

unxfuncs.$(OBJ) :$(SRCDIR)/unxfuncs.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/unxfuncs.c

arxfuncs.$(OBJ) :$(SRCDIR)/arxfuncs.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/arxfuncs.c

os2funcs.$(OBJ) :$(SRCDIR)/os2funcs.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/os2funcs.c

cmsfuncs.$(OBJ) :$(SRCDIR)/cmsfuncs.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/cmsfuncs.c

shell.$(OBJ) :	 $(SRCDIR)/shell.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/shell.c

rexxext.$(OBJ) : $(SRCDIR)/rexxext.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/rexxext.c

stack.$(OBJ) :	 $(SRCDIR)/stack.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/stack.c

tracing.$(OBJ) : $(SRCDIR)/tracing.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/tracing.c

interp.$(OBJ) :	 $(SRCDIR)/interp.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/interp.c

cmath.$(OBJ) :	 $(SRCDIR)/cmath.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/cmath.c

convert.$(OBJ) : $(SRCDIR)/convert.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/convert.c

strings.$(OBJ) : $(SRCDIR)/strings.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/strings.c

library.$(OBJ) : $(SRCDIR)/library.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/library.c

strmath.$(OBJ) : $(SRCDIR)/strmath.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/strmath.c

signals.$(OBJ) : $(SRCDIR)/signals.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/signals.c

macros.$(OBJ) :	 $(SRCDIR)/macros.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/macros.c

envir.$(OBJ) :	 $(SRCDIR)/envir.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/envir.c

expr.$(OBJ) :	 $(SRCDIR)/expr.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/expr.c

wrappers.$(OBJ) :$(SRCDIR)/wrappers.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/wrappers.c

options.$(OBJ) : $(SRCDIR)/options.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/options.c

os_unx.$(OBJ) :	 $(SRCDIR)/os_unx.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/os_unx.c

rexx.$(OBJ) :	 $(SRCDIR)/rexx.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/rexx.c

client.$(OBJ) :	   $(SRCDIR)/client.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/client.c

alloca.$(OBJ) :	   $(SRCDIR)/alloca.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/alloca.c

mt_notmt.$(OBJ) :    $(SRCDIR)/mt_notmt.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/mt_notmt.c

nosaa.$(OBJ) :	  $(SRCDIR)/nosaa.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/nosaa.c

instore.$(OBJ) : $(SRCDIR)/instore.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/instore.c

extstack.$(OBJ) : $(srcdir)/extstack.c $(srcdir)/extstack.h $(srcdir)/rexxbif.h
	$(CC) $(COPT) $(CC2O) -c $(srcdir)/extstack.c

rexxbif.$(OBJ) : $(srcdir)/rexxbif.c $(srcdir)/rexxbif.h
	$(CC) $(COPT) $(CC2O) $(srcdir)/rexxbif.c

rxstack.$(OBJ) : $(srcdir)/rxstack.c $(srcdir)/rexxbif.h
	$(CC) DEF EXTERNAL_TO_REGINA $(COPT) $(CC2O) $(srcdir)/rxstack.c DEF HAVE_SYS_SOCKET_H DEF HAVE_NETINET_IN_H IDIR netinclude: IGN 51

rxqueue.$(OBJ) : $(srcdir)/rxqueue.c $(srcdir)/rexxbif.h
	$(CC) DEF EXTERNAL_TO_REGINA $(COPT) $(CC2O) $(srcdir)/rxqueue.c

eextstack.$(OBJ) : $(srcdir)/extstack.c $(srcdir)/extstack.h $(srcdir)/rexxbif.h
	$(CC) DEF EXTERNAL_TO_REGINA $(COPT) $(CC2O) $(srcdir)/extstack.c DEF HAVE_SYS_SOCKET_H DEF HAVE_NETINET_IN_H DEF HAVE_NETDB_H IDIR netinclude:
	$(MV) extstack.o $@

erexxbif.$(OBJ) : $(srcdir)/rexxbif.c $(srcdir)/rexxbif.h
	$(CC) DEF EXTERNAL_TO_REGINA $(COPT) $(CC2O) TO=$@ $(srcdir)/rexxbif.c
	$(MV) rexxbif.o $@

rexxsaa.$(OBJ) :    $(SRCDIR)/rexxsaa.c
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/rexxsaa.c

regina.$(OBJ) :	 $(SRCDIR)/regina.c
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/regina.c

drexx.$(OBJ) : $(SRCDIR)/rexx.c
	$(MV) rexx.$(OBJ) rexx$(OSAVE)
	$(CC) $(COPT) DEF RXLIB $(srcdir)/rexx.c
	$(MV) rexx.$(OBJ) drexx.$(OBJ)
	$(MV) rexx$(OSAVE) rexx.$(OBJ)

lexsrc.$(OBJ) : $(SRCDIR)/lexsrc.c $(SRCDIR)/rexx.h $(SRCDIR)/yaccsrc.h
	$(CC) DATA=f $(OTHERCOPT) $(CC2O) $(SRCDIR)/lexsrc.c

yaccsrc.$(OBJ) : $(SRCDIR)/yaccsrc.c $(SRCDIR)/defs.h $(SRCDIR)/rexx.h
	$(CC) $(OTHERCOPT) $(CC2O) DEF YYMAXDEPTH=10000 $(SRCDIR)/yaccsrc.c #DEF alloca=malloc

execiser.$(OBJ) : $(SRCDIR)/execiser.c $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) $(SRCDIR)/execiser.c

#
# Rules for shared objects
#
funcs.sho :   $(SRCDIR)/funcs.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/funcs.c
	$(O2SHO)
	$(SAVE2O)

builtin.sho : $(SRCDIR)/builtin.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/builtin.c
	$(O2SHO)
	$(SAVE2O)

error.sho :   $(SRCDIR)/error.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/error.c
	$(O2SHO)
	$(SAVE2O)

variable.sho :$(SRCDIR)/variable.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/variable.c
	$(O2SHO)
	$(SAVE2O)

interprt.sho :$(SRCDIR)/interprt.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/interprt.c
	$(O2SHO)
	$(SAVE2O)

debug.sho :   $(SRCDIR)/debug.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/debug.c
	$(O2SHO)
	$(SAVE2O)

dbgfuncs.sho :$(SRCDIR)/dbgfuncs.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/dbgfuncs.c
	$(O2SHO)
	$(SAVE2O)

memory.sho :  $(SRCDIR)/memory.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/memory.c
	$(O2SHO)
	$(SAVE2O)

parsing.sho : $(SRCDIR)/parsing.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/parsing.c
	$(O2SHO)
	$(SAVE2O)

files.sho :   $(SRCDIR)/files.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/files.c
	$(O2SHO)
	$(SAVE2O)

misc.sho :    $(SRCDIR)/misc.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/misc.c
	$(O2SHO)
	$(SAVE2O)

unxfuncs.sho :$(SRCDIR)/unxfuncs.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/unxfuncs.c
	$(O2SHO)
	$(SAVE2O)

arxfuncs.sho :$(SRCDIR)/arxfuncs.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/arxfuncs.c
	$(O2SHO)
	$(SAVE2O)

os2funcs.sho :$(SRCDIR)/os2funcs.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/os2funcs.c
	$(O2SHO)
	$(SAVE2O)

cmsfuncs.sho :$(SRCDIR)/cmsfuncs.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/cmsfuncs.c
	$(O2SHO)
	$(SAVE2O)

shell.sho :   $(SRCDIR)/shell.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/shell.c
	$(O2SHO)
	$(SAVE2O)

rexxext.sho : $(SRCDIR)/rexxext.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/rexxext.c
	$(O2SHO)
	$(SAVE2O)

stack.sho :   $(SRCDIR)/stack.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/stack.c
	$(O2SHO)
	$(SAVE2O)

tracing.sho : $(SRCDIR)/tracing.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/tracing.c
	$(O2SHO)
	$(SAVE2O)

interp.sho :  $(SRCDIR)/interp.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/interp.c
	$(O2SHO)
	$(SAVE2O)

cmath.sho :   $(SRCDIR)/cmath.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/cmath.c
	$(O2SHO)
	$(SAVE2O)

convert.sho : $(SRCDIR)/convert.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/convert.c
	$(O2SHO)
	$(SAVE2O)

strings.sho : $(SRCDIR)/strings.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/strings.c
	$(O2SHO)
	$(SAVE2O)

library.sho : $(SRCDIR)/library.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/library.c
	$(O2SHO)
	$(SAVE2O)

strmath.sho : $(SRCDIR)/strmath.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/strmath.c
	$(O2SHO)
	$(SAVE2O)

signals.sho : $(SRCDIR)/signals.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/signals.c
	$(O2SHO)
	$(SAVE2O)

macros.sho :  $(SRCDIR)/macros.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/macros.c
	$(O2SHO)
	$(SAVE2O)

envir.sho :   $(SRCDIR)/envir.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/envir.c
	$(O2SHO)
	$(SAVE2O)

expr.sho :    $(SRCDIR)/expr.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/expr.c
	$(O2SHO)
	$(SAVE2O)

wrappers.sho :$(SRCDIR)/wrappers.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/wrappers.c
	$(O2SHO)
	$(SAVE2O)

options.sho : $(SRCDIR)/options.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/options.c
	$(O2SHO)
	$(SAVE2O)

os_unx.sho :  $(SRCDIR)/os_unx.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/os_unx.c
	$(O2SHO)
	$(SAVE2O)

rexx.sho :  $(SRCDIR)/rexx.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/rexx.c
	$(O2SHO)
	$(SAVE2O)

client.sho :  $(SRCDIR)/client.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/client.c
	$(O2SHO)
	$(SAVE2O)

alloca.sho :  $(SRCDIR)/alloca.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/alloca.c
	$(O2SHO)
	$(SAVE2O)

rexxsaa.sho :  $(SRCDIR)/rexxsaa.c
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/rexxsaa.c
	$(O2SHO)
	$(SAVE2O)

drexx.sho : $(SRCDIR)/rexx.c
	$(MV) rexx.$(OBJ) rexx.$(OBJ).save
	$(CC) $(COPT) $(DYN_COMP) -c -DRXLIB $(SRCDIR)/rexx.c
	$(MV) rexx.$(OBJ) drexx.sho
	$(MV) rexx.$(OBJ).save rexx.$(OBJ)

lexsrc.sho : $(SRCDIR)/lexsrc.c $(SRCDIR)/rexx.h $(SRCDIR)/yaccsrc.h
	$(O2SAVE)
	$(CC) $(OTHERCOPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/lexsrc.c
	$(O2SHO)
	$(SAVE2O)

yaccsrc.sho : $(SRCDIR)/yaccsrc.c $(SRCDIR)/defs.h $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(OTHERCOPT) $(CC2O) $(DYN_COMP) -c -DYYMAXDEPTH=10000 $(SRCDIR)/yaccsrc.c
	$(O2SHO)
	$(SAVE2O)

#
# Rules for sample programs
#
test1.$(OBJ): $(SRCDIR)/test1.c
	$(CC) -c $(COPT) $(CC2O) $(DYN_COMP) $(SRCDIR)/test1.c

rxtest1.$(RXLIB): test1.$(OBJ) $(LIBPRE)$(SHLFILE).$(SHL)
	$(LD_RXLIB1) test1.$(OBJ) $(LD_RXLIB2)

test2.$(OBJ): $(SRCDIR)/test2.c
	$(CC) -c $(COPT) $(CC2O) $(DYN_COMP) $(SRCDIR)/test2.c

rxtest2.$(RXLIB): test2.$(OBJ) $(LIBPRE)$(SHLFILE).$(SHL)
	$(LD_RXLIB1) test2.$(OBJ) $(LD_RXLIB2)

depend:
	makedepend $(CSRCFILES)

ARCHIVE_FILES = $(ALLSRC) $(README) $(SRCDIR)/makefile* $(SRCDIR)/*.rsp $(DEMOFILES) $(TRIPFILES)

dist:
	tar -cvf - $(ARCHIVE_FILES) | gzip > Regina_$(VERDOT).tar.gz

tar:
	tar -cvf - $(ARCHIVE_FILES) | compress > Regina_$(VERDOT).tar.Z

zip:
	zip regna$(VER) $(ARCHIVE_FILES)

#
# below here not tested
#

install: rexx $(LIBPRE)$(LIBFILE).a
	cp rexx$(PROGEXT) $(BINDIR)/$(PROGNAME)$(PROGEXT)
	cp ../regina.1 $(MANDIR)/regina.$(MANEXT)
	cp $(LIBPRE)$(LIBFILE).a $(LIBDIR)/$(LIBPRE)$(LIBFILE).a
	ranlib $(LIBDIR)/$(LIBPRE)$(LIBFILE).a

clean:
	delete FORCE $(JUNK)
#
# End of makefile
#
