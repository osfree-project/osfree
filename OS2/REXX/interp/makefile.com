#
# Some settings	that you may wish to change, these are used only
# for the 'install' target:
#
#   TOPDIR    The top of the directory hierarchy in which to install
#   BINDIR    Where to install the compiled 'rexx' program
#   LIBDIR    Where to install the 'regina' library
#   MANEXT    The extention of the manual page
#   MANDIR    Where to install the manual page
#   PROGNAME  What to call the compiled	program
#
.SUFFIXES: .l .y
TOPDIR = /usr/local
BINDIR = $(TOPDIR)/bin
LIBDIR = $(TOPDIR)/lib
MANEXT = l
MANDIR = $(TOPDIR)/man/man$(MANEXT)

include $(SRCDIR)\regina.ver
VERDOT = $(VER_DOT)
VERDATE = $(VER_DATE)

MYCEXTRA = $(CEXTRA) -DREGINA_VERSION_DATE=\"$(VER_DATE)\" -DREGINA_VERSION_MAJOR=\"$(VER_MAJOR)\" -DREGINA_VERSION_MINOR=\"$(VER_MINOR)\" -DREGINA_VERSION_SUPP=\"$(VER_SUPP)\"

MISCDEFS = -I$(SRCDIR) #-DR2PERL
#DEBUG	  = #-g	-DNDEBUG #-Dlint
PROF	 = #-p #g
#OPTIMIZE = -O -DNDEBUG

COPT = $(DEBUGFLAG) $(MYCEXTRA) $(OPTIMIZE) $(PROF) $(MISCDEFS) $(MEMTRACE) $(USEFLISTS) $(THREADING)
LINKOPT	= $(DEBUGFLAG) $(LDOPT) $(PROF) $(EEXTRA)
OTHERCOPT = $(DEBUGFLAG) $(OPTIMIZE) $(PROF) $(MYCEXTRA) $(MISCDEFS) $(MEMTRACE) $(USEFLISTS) $(THREADING)

YACC = yacc
#YACC =	bison -ly #-d #-lyt

LEX = lex
#LEX = flex -8 #-d
#LL = -L/store/lib -lfl

README = $(SRCDIR)/README.* COPYING-LIB

#FILES =	funcs builtin error variable interprt debug dbgfuncs \
#	memory parsing files misc os2funcs unxfuncs cmsfuncs shell rexxext stack \
#	tracing	interp cmath convert strings library strmath signals \
#	macros envir wrappers options $(OS_FILE) extstack rexxbif

OFILES = funcs.$(OBJ) builtin.$(OBJ) error.$(OBJ) variable.$(OBJ) \
	interprt.$(OBJ)	debug.$(OBJ) dbgfuncs.$(OBJ) memory.$(OBJ) parsing.$(OBJ) files.$(OBJ) \
	misc.$(OBJ) unxfuncs.$(OBJ) arxfuncs.$(OBJ) cmsfuncs.$(OBJ) os2funcs.$(OBJ) shell.$(OBJ) rexxext.$(OBJ) stack.$(OBJ) \
	tracing.$(OBJ) interp.$(OBJ) cmath.$(OBJ) convert.$(OBJ) strings.$(OBJ)	library.$(OBJ) \
	strmath.$(OBJ) signals.$(OBJ) macros.$(OBJ) envir.$(OBJ) expr.$(OBJ) $(MT_FILE).$(OBJ) instore.$(OBJ) \
	yaccsrc.$(OBJ) lexsrc.$(OBJ) options.$(OBJ) $(OS_FILE).$(OBJ) wrappers.$(OBJ) \
	$(EXTQUEUE_OBJS)

SHOFILES = funcs.sho builtin.sho error.sho variable.sho	\
	interprt.sho debug.sho dbgfuncs.sho memory.sho parsing.sho files.sho \
	misc.sho unxfuncs.sho arxfuncs.sho cmsfuncs.sho shell.sho os2funcs.sho rexxext.sho stack.sho	\
	tracing.sho interp.sho cmath.sho convert.sho strings.sho library.sho \
	strmath.sho signals.sho	macros.sho envir.sho expr.sho $(MT_FILE).sho instore.sho \
	yaccsrc.sho lexsrc.sho wrappers.sho options.sho $(OS_FILE).sho \
	drexx.sho client.sho rexxsaa.sho \
	$(EXTQUEUE_SHOBJS)

DEMODIR	= $(SRCDIR)/demo
DEMOFILES = $(DEMODIR)/*
TRIPDIR	= $(SRCDIR)/trip
TRIPFILES = $(TRIPDIR)/*

ALLSRC = $(CFILES) $(HFILES)

HFILES = $(SRCDIR)/rexx.h $(SRCDIR)/defs.h $(SRCDIR)/extern.h $(SRCDIR)/regina_t.h	\
		$(SRCDIR)/config.h $(SRCDIR)/strings.h $(SRCDIR)/configur.h $(SRCDIR)/yaccsrc.h	\
		$(SRCDIR)/rxiface.h $(SRCDIR)/rexxsaa.h	$(SRCDIR)/utsname.h

CFILES = $(SRCDIR)/yaccsrc.c $(SRCDIR)/lexsrc.c	$(CSRCFILES) $(SRCDIR)/execiser.c \
		$(SRCDIR)/rexxsaa.c $(SRCDIR)/rexx.c $(SRCDIR)/regina.c	$(SRCDIR)/client.c \
		$(SRCDIR)/r2perl.c $(SRCDIR)/test1.c $(SRCDIR)/test2.c

MTSSRC = $(SRCDIR)/en.mts $(SRCDIR)/pt.mts $(SRCDIR)/no.mts \
	$(SRCDIR)/de.mts $(SRCDIR)/es.mts $(SRCDIR)/pl.mts \
	$(SRCDIR)/tr.mts $(SRCDIR)/sv.mts

#.SUFFIXES: .sho $(SUFFIXES)
#
# Do not use default suffixes rule
#
.SUFFIXES:

all : rexx$(EXE) execiser$(EXE) $(SHL_TARGETS) mtb_files

$(CSRCFILES) : $(SRCDIR)/rexx.h

$(SRCDIR)/rexx.h : $(SRCDIR)/extern.h $(SRCDIR)/strings.h $(SRCDIR)/defs.h $(SRCDIR)/regina_t.h $(SRCDIR)/mt.h

$(SRCDIR)/mt.h : $(SRCDIR)/$(MT_FILE).h

rexx$(EXE) : $(OFILES) rexx.$(OBJ) nosaa.$(OBJ)
	$(PURIFY) $(LD) $(LINKOPT) $(STATIC_LDFLAGS) -o rexx$(EXE) $(OBJECTS) rexx.$(OBJ) nosaa.$(OBJ) $(OS2LIBA) $(LIBS) $(TCPLIBS)
	$(LDEXTRA)
	$(LDEXTRA1)

regina$(EXE) : $(LIBPRE)$(SHLFILE).$(SHL) regina.$(OBJ)	$(SHLIMPLIB) $(OS2LIBA)
	$(PURIFY) $(LD)	$(LINKOPT) $(DYNAMIC_LDFLAGS) -o regina$(EXE) regina.$(OBJ) $(LINKSHL) $(LIBS) $(SHLIBS) $(OS2LIBA)
	$(LDEXTRA)

execiser$(EXE) : execiser.$(OBJ) $(LIBPRE)$(LIBFILE).$(LIBPST) $(OS2LIBA)
	$(PURIFY) $(LD)	$(LINKOPT) -o execiser$(EXE) execiser.$(OBJ) $(LIBLINK) $(OS2LIBA) $(TCPLIBS)
	$(LDEXTRA)
	$(LDEXTRA1)

threader$(EXE) : threader.$(OBJ) $(LIBPRE)$(LIBFILE).$(LIBPST) $(OS2LIBA)
	$(PURIFY) $(LD)	$(LINKOPT) -o threader$(EXE) threader.$(OBJ) $(LIBLINK) $(OS2LIBA)
	$(LDEXTRA)
	$(LDEXTRA1)

rxstack$(EXE) : rxstack.$(OBJ) erexxbif.$(OBJ) eextstack.$(OBJ)
	$(PURIFY) $(LD) $(LINKOPT) $(STATIC_LDFLAGS) -o rxstack$(EXE) rxstack.$(OBJ) erexxbif.$(OBJ) eextstack.$(OBJ) $(LIBS) $(TCPLIBS)
	$(LDEXTRA)
	$(LDEXTRA1)

rxqueue$(EXE) : rxqueue.$(OBJ) erexxbif.$(OBJ) eextstack.$(OBJ)
	$(PURIFY) $(LD) $(LINKOPT) $(STATIC_LDFLAGS) -o rxqueue$(EXE) rxqueue.$(OBJ) erexxbif.$(OBJ) eextstack.$(OBJ) $(LIBS) $(TCPLIBS)
	$(LDEXTRA)
	$(LDEXTRA1)

$(LIBPRE)$(LIBFILE).$(LIBPST): $(OFILES) client.$(OBJ) drexx.$(OBJ) rexxsaa.$(OBJ)
	$(LIBEXE) $(LIBFLAGS) $(OBJECTS) client.$(OBJ) drexx.$(OBJ) rexxsaa.$(OBJ)
	$(RANLIB_LIB)

$(LIBPRE)$(SHLFILE).$(SHL): $(SHOFILES) $(SHLDEF) $(OS2LIB)
	$(SHL_LD)
	$(SHL1)
	$(SHL2)
	$(SHL3)
	$(SHL4)

$(LIBPRE)rxtest1.$(SHL): $(LIBPRE)$(SHLFILE).$(SHL) $(TEST1DEF) test1.sho
	$(TEST1_LD)
	$(TEST1_SHL1)

$(LIBPRE)rxtest2.$(SHL): $(LIBPRE)$(SHLFILE).$(SHL) $(TEST2DEF) test2.sho
	$(TEST2_LD)
	$(TEST2_SHL1)

$(OS2LIB):
	$(PURGEOS2LIB1)
	$(PURGEOS2LIB2)

$(OS2LIBA):
	$(PURGEOS2LIB1)
	$(PURGEOS2LIB2)

#
# Convert .mts to .mtb
#
mtb_files: msgcmp$(EXE) $(MTSSRC)
	-msgcmp$(EXE) $(MTSSRC)

msgcmp$(EXE): msgcmp.$(OBJ)
	$(PURIFY) $(LD) $(LINKOPT) $(STATIC_LDFLAGS) -o msgcmp$(EXE) msgcmp.$(OBJ) $(OS2LIBA) $(LIBS)
	$(LDEXTRA)
	$(LDEXTRA1)

msgcmp.$(OBJ): $(SRCDIR)/msgcmp.c
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/msgcmp.c

#
# Rules	for static objects
#
#$(SRCDIR)/lexsrc.c: $(SRCDIR)/lexsrc.l
#	flex -Scontrib/flex.skl	-8 -o$(SRCDIR)/lexsrc.c	$(SRCDIR)/lexsrc.l

#$(SRCDIR)/yaccsrc.c $(SRCDIR)/yaccsrc.h: $(SRCDIR)/yaccsrc.y
#	bison -d -o$(SRCDIR)/yaccsrc.c $(SRCDIR)/yaccsrc.y

funcs.$(OBJ) :	 $(SRCDIR)/funcs.c	 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/funcs.c

builtin.$(OBJ) : $(SRCDIR)/builtin.c	 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/builtin.c

error.$(OBJ) :	 $(SRCDIR)/error.c	 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/error.c

variable.$(OBJ)	:$(SRCDIR)/variable.c	 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/variable.c

interprt.$(OBJ)	:$(SRCDIR)/interprt.c	 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/interprt.c

debug.$(OBJ) :	 $(SRCDIR)/debug.c	 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/debug.c

dbgfuncs.$(OBJ)	:$(SRCDIR)/dbgfuncs.c	 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/dbgfuncs.c

memory.$(OBJ) :	 $(SRCDIR)/memory.c	 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/memory.c

parsing.$(OBJ) : $(SRCDIR)/parsing.c	 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/parsing.c

files.$(OBJ) :	 $(SRCDIR)/files.c	 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/files.c

misc.$(OBJ) :	 $(SRCDIR)/misc.c	 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/misc.c

unxfuncs.$(OBJ)	:$(SRCDIR)/unxfuncs.c	 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/unxfuncs.c

arxfuncs.$(OBJ)	:$(SRCDIR)/arxfuncs.c	 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/arxfuncs.c

os2funcs.$(OBJ)	:$(SRCDIR)/os2funcs.c	 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/os2funcs.c

cmsfuncs.$(OBJ)	:$(SRCDIR)/cmsfuncs.c	 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/cmsfuncs.c

shell.$(OBJ) :	 $(SRCDIR)/shell.c	 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/shell.c

rexxext.$(OBJ) : $(SRCDIR)/rexxext.c	 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/rexxext.c

stack.$(OBJ) :	 $(SRCDIR)/stack.c	 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/stack.c

extstack.$(OBJ) :	 $(SRCDIR)/extstack.c	 $(SRCDIR)/rexx.h $(SRCDIR)/extstack.h $(SRCDIR)/rexxbif.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/extstack.c

rexxbif.$(OBJ) :	 $(SRCDIR)/rexxbif.c	 $(SRCDIR)/rexxbif.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/rexxbif.c

tracing.$(OBJ) : $(SRCDIR)/tracing.c	 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/tracing.c

interp.$(OBJ) :	 $(SRCDIR)/interp.c	 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/interp.c

cmath.$(OBJ) :	 $(SRCDIR)/cmath.c	 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/cmath.c

convert.$(OBJ) : $(SRCDIR)/convert.c	 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/convert.c

strings.$(OBJ) : $(SRCDIR)/strings.c	 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/strings.c

library.$(OBJ) : $(SRCDIR)/library.c	 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/library.c

strmath.$(OBJ) : $(SRCDIR)/strmath.c	 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/strmath.c

signals.$(OBJ) : $(SRCDIR)/signals.c	 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/signals.c

macros.$(OBJ) :	 $(SRCDIR)/macros.c	 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/macros.c

envir.$(OBJ) :	 $(SRCDIR)/envir.c	 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/envir.c

expr.$(OBJ) :	 $(SRCDIR)/expr.c	 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/expr.c

wrappers.$(OBJ)	:$(SRCDIR)/wrappers.c	 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/wrappers.c

options.$(OBJ) : $(SRCDIR)/options.c	 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/options.c

$(OS_FILE).$(OBJ) :	 $(SRCDIR)/$(OS_FILE).c $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/$(OS_FILE).c

rexx.$(OBJ) :	 $(SRCDIR)/rexx.c	 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/rexx.c

client.$(OBJ) :	   $(SRCDIR)/client.c	 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/client.c

rexxsaa.$(OBJ) :    $(SRCDIR)/rexxsaa.c	 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/rexxsaa.c

regina.$(OBJ) :	 $(SRCDIR)/regina.c	 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/regina.c

drexx.$(OBJ) : $(SRCDIR)/rexx.c		 $(SRCDIR)/rexx.h
	$(CC) $(COPT) -c -o drexx.$(OBJ) -DRXLIB $(SRCDIR)/rexx.c

mt_notmt.$(OBJ) : $(SRCDIR)/mt_notmt.c
	$(CC) $(COPT) -c $(CC2O) $(SRCDIR)/mt_notmt.c

mt_win32.$(OBJ) : $(SRCDIR)/mt_win32.c
	$(CC) $(COPT) -c $(CC2O) $(SRCDIR)/mt_win32.c

mt_os2.$(OBJ) : $(SRCDIR)/mt_os2.c
	$(CC) $(COPT) -c $(CC2O) $(SRCDIR)/mt_os2.c

instore.$(OBJ) : $(SRCDIR)/instore.c
	$(CC) $(COPT) -c $(CC2O) $(SRCDIR)/instore.c

nosaa.$(OBJ) : $(SRCDIR)/nosaa.c
	$(CC) $(COPT) -c $(CC2O) $(SRCDIR)/nosaa.c

lexsrc.$(OBJ) :	$(SRCDIR)/lexsrc.c $(SRCDIR)/rexx.h $(SRCDIR)/yaccsrc.h
	$(CC) $(OTHERCOPT) $(CC2O) -c $(SRCDIR)/lexsrc.c

yaccsrc.$(OBJ) : $(SRCDIR)/yaccsrc.c $(SRCDIR)/defs.h $(SRCDIR)/rexx.h
	$(CC) $(OTHERCOPT) $(CC2O) -c -DYYMAXDEPTH=10000 $(SRCDIR)/yaccsrc.c

execiser.$(OBJ)	: $(SRCDIR)/execiser.c $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) -c $(SRCDIR)/execiser.c

threader.$(OBJ)	: $(SRCDIR)/threader.c $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(CC2O) $(THREAD_TYPE) -c $(SRCDIR)/threader.c

eextstack.$(OBJ) :	 $(SRCDIR)/extstack.c	 $(SRCDIR)/rexx.h $(SRCDIR)/rexxbif.h $(SRCDIR)/extstack.h
	$(CC) -DEXTERNAL_TO_REGINA  $(COPT) $(CC2O) -c $(SRCDIR)/extstack.c

erexxbif.$(OBJ) :	 $(SRCDIR)/rexxbif.c	 $(SRCDIR)/rexxbif.h
	$(CC) -DEXTERNAL_TO_REGINA $(COPT) $(CC2O) -c $(SRCDIR)/rexxbif.c

rxqueue.$(OBJ) :	 $(SRCDIR)/rxqueue.c	 $(SRCDIR)/rexx.h $(SRCDIR)/rexxbif.h $(SRCDIR)/extstack.h
	$(CC) -DEXTERNAL_TO_REGINA $(COPT) $(CC2O) -c $(SRCDIR)/rxqueue.c

rxstack.$(OBJ) :	 $(SRCDIR)/rxstack.c	 $(SRCDIR)/rexx.h $(SRCDIR)/rexxbif.h $(SRCDIR)/extstack.h
	$(CC) -DEXTERNAL_TO_REGINA $(COPT) $(CC2O) -c $(SRCDIR)/rxstack.c

#
# Rules	for shared objects
#
funcs.sho :   $(SRCDIR)/funcs.c		 $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/funcs.c
	$(O2SHO)
	$(SAVE2O)

builtin.sho : $(SRCDIR)/builtin.c	 $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/builtin.c
	$(O2SHO)
	$(SAVE2O)

error.sho :   $(SRCDIR)/error.c		 $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/error.c
	$(O2SHO)
	$(SAVE2O)

variable.sho :$(SRCDIR)/variable.c	 $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/variable.c
	$(O2SHO)
	$(SAVE2O)

interprt.sho :$(SRCDIR)/interprt.c	 $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/interprt.c
	$(O2SHO)
	$(SAVE2O)

debug.sho :   $(SRCDIR)/debug.c		 $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/debug.c
	$(O2SHO)
	$(SAVE2O)

dbgfuncs.sho :$(SRCDIR)/dbgfuncs.c	 $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/dbgfuncs.c
	$(O2SHO)
	$(SAVE2O)

memory.sho :  $(SRCDIR)/memory.c	 $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/memory.c
	$(O2SHO)
	$(SAVE2O)

parsing.sho : $(SRCDIR)/parsing.c	 $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/parsing.c
	$(O2SHO)
	$(SAVE2O)

files.sho :   $(SRCDIR)/files.c		 $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/files.c
	$(O2SHO)
	$(SAVE2O)

misc.sho :    $(SRCDIR)/misc.c		 $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/misc.c
	$(O2SHO)
	$(SAVE2O)

unxfuncs.sho :$(SRCDIR)/unxfuncs.c	 $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/unxfuncs.c
	$(O2SHO)
	$(SAVE2O)

arxfuncs.sho :$(SRCDIR)/arxfuncs.c	 $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/arxfuncs.c
	$(O2SHO)
	$(SAVE2O)

os2funcs.sho :$(SRCDIR)/os2funcs.c	 $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/os2funcs.c
	$(O2SHO)
	$(SAVE2O)

cmsfuncs.sho :$(SRCDIR)/cmsfuncs.c	 $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/cmsfuncs.c
	$(O2SHO)
	$(SAVE2O)

shell.sho :   $(SRCDIR)/shell.c		 $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/shell.c
	$(O2SHO)
	$(SAVE2O)

rexxext.sho : $(SRCDIR)/rexxext.c	 $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/rexxext.c
	$(O2SHO)
	$(SAVE2O)

stack.sho :   $(SRCDIR)/stack.c		 $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/stack.c
	$(O2SHO)
	$(SAVE2O)

extstack.sho :$(SRCDIR)/extstack.c		 $(SRCDIR)/rexx.h $(SRCDIR)/extstack.h $(SRCDIR)/rexxbif.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/extstack.c
	$(O2SHO)
	$(SAVE2O)

rexxbif.sho :$(SRCDIR)/rexxbif.c  $(SRCDIR)/rexxbif.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/rexxbif.c
	$(O2SHO)
	$(SAVE2O)

tracing.sho : $(SRCDIR)/tracing.c	 $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/tracing.c
	$(O2SHO)
	$(SAVE2O)

interp.sho :  $(SRCDIR)/interp.c	 $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/interp.c
	$(O2SHO)
	$(SAVE2O)

cmath.sho :   $(SRCDIR)/cmath.c		 $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/cmath.c
	$(O2SHO)
	$(SAVE2O)

convert.sho : $(SRCDIR)/convert.c	 $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/convert.c
	$(O2SHO)
	$(SAVE2O)

strings.sho : $(SRCDIR)/strings.c	 $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/strings.c
	$(O2SHO)
	$(SAVE2O)

library.sho : $(SRCDIR)/library.c	 $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/library.c
	$(O2SHO)
	$(SAVE2O)

strmath.sho : $(SRCDIR)/strmath.c	 $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/strmath.c
	$(O2SHO)
	$(SAVE2O)

signals.sho : $(SRCDIR)/signals.c	 $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/signals.c
	$(O2SHO)
	$(SAVE2O)

macros.sho :  $(SRCDIR)/macros.c	 $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/macros.c
	$(O2SHO)
	$(SAVE2O)

envir.sho :   $(SRCDIR)/envir.c		 $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/envir.c
	$(O2SHO)
	$(SAVE2O)

expr.sho :    $(SRCDIR)/expr.c		 $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/expr.c
	$(O2SHO)
	$(SAVE2O)

instore.sho :    $(SRCDIR)/instore.c		 $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/instore.c
	$(O2SHO)
	$(SAVE2O)

wrappers.sho :$(SRCDIR)/wrappers.c	 $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/wrappers.c
	$(O2SHO)
	$(SAVE2O)

options.sho : $(SRCDIR)/options.c	 $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/options.c
	$(O2SHO)
	$(SAVE2O)

$(OS_FILE).sho :  $(SRCDIR)/$(OS_FILE).c $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/$(OS_FILE).c
	$(O2SHO)
	$(SAVE2O)

rexx.sho :  $(SRCDIR)/rexx.c		 $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/rexx.c
	$(O2SHO)
	$(SAVE2O)

client.sho :  $(SRCDIR)/client.c	 $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/client.c
	$(O2SHO)
	$(SAVE2O)

rexxsaa.sho :  $(SRCDIR)/rexxsaa.c	 $(SRCDIR)/rexx.h
	$(O2SAVE)
	$(CC) $(COPT) $(CC2O) $(DYN_COMP) -c $(SRCDIR)/rexxsaa.c
	$(O2SHO)
	$(SAVE2O)

drexx.sho : $(SRCDIR)/rexx.c		 $(SRCDIR)/rexx.h
	$(CC) $(COPT) $(DYN_COMP) -c -o drexx.sho -DRXLIB $(SRCDIR)/rexx.c

mt_notmt.sho :$(SRCDIR)/mt_notmt.c
	$(O2SAVE)
	$(CC) $(COPT) $(DYN_COMP) -c -o mt_notmt.sho $(SRCDIR)/mt_notmt.c
	$(O2SHO)
	$(SAVE2O)

mt_win32.obj :$(SRCDIR)/mt_win32.c
	$(O2SAVE)
	$(CC) $(COPT) $(DYN_COMP) -c $(SRCDIR)/mt_win32.c
	$(O2SHO)
	$(SAVE2O)

mt_os2.obj :$(SRCDIR)/mt_os2.c
	$(O2SAVE)
	$(CC) $(COPT) $(DYN_COMP) -c $(SRCDIR)/mt_os2.c
	$(O2SHO)
	$(SAVE2O)

lexsrc.sho : $(SRCDIR)/lexsrc.c	$(SRCDIR)/rexx.h $(SRCDIR)/yaccsrc.h
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

test1.sho: $(SRCDIR)/test1.c
	$(CC) -c $(COPT) $(CC2O) $(DYN_COMP) $(SRCDIR)/test1.c

rxtest1.$(RXLIB):	test1.$(OBJ) $(LIBPRE)$(SHLFILE).$(SHL)
	$(LD_RXLIB_PRE)	test1.$(OBJ) $(LD_RXLIB1)

test2.$(OBJ): $(SRCDIR)/test2.c
	$(CC) -c $(COPT) $(CC2O) $(DYN_COMP) $(SRCDIR)/test2.c

test2.sho: $(SRCDIR)/test2.c
	$(CC) -c $(COPT) $(CC2O) $(DYN_COMP) $(SRCDIR)/test2.c

rxtest2.$(RXLIB):	test2.$(OBJ) $(LIBPRE)$(SHLFILE).$(SHL)
	$(LD_RXLIB_PRE)	test2.$(OBJ) $(LD_RXLIB2)

depend:
	makedepend $(CSRCFILES)

ARCHIVE_FILES =	$(ALLSRC) $(README) $(SRCDIR)/makefile*	$(SRCDIR)/*.rsp	$(DEMOFILES) $(TRIPFILES)

dist:
	tar -cvf - $(ARCHIVE_FILES) | gzip > Regina_$(VERDOT).tar.gz

tar:
	tar -cvf - $(ARCHIVE_FILES) | compress > Regina_$(VERDOT).tar.Z

zip:
	zip regna$(VER)	$(ARCHIVE_FILES)

#
# below	here not tested
#

install: rexx $(LIBPRE)$(LIBFILE).a
	cp rexx$(PROGEXT) $(BINDIR)/$(PROGNAME)$(PROGEXT)
	cp ../regina.1 $(MANDIR)/regina.$(MANEXT)
	cp $(LIBPRE)$(LIBFILE).a $(LIBDIR)/$(LIBPRE)$(LIBFILE).a
	ranlib $(LIBDIR)/$(LIBPRE)$(LIBFILE).a

clean:
	rm -f $(JUNK) \#*\#
#
# End of makefile
#
