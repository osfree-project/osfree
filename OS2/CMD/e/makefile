#
# A Makefile for E
# (c) osFree project,
# author, date
#

PROJ1        = e
TRGT         = $(PROJ1).exe
PROJ         = e0
DESC         = osFree System Editor
DIRS         = # help
#defines object file names in format objname.$(O)
srcfiles     = $(p)sty_main$(e) $(p)sty_user$(e) $(p)sty_init$(e) &
               $(p)sty_pnt$(e)  $(p)sty_dlg$(e)  $(p)sty_file$(e) &
               $(p)sty_edit$(e) $(p)sty_help$(e)
# defines additional options for C compiler
ADD_COPT     = -i=$(MYDIR) -i=$(%WATCOM)$(SEP)h$(SEP)os2 -sg -3s # until UniAPI modules will be done
ADD_LINKOPT  = lib os2386.lib, clib3s.lib
# STUB       = $(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe
PM           = 1
RES          = sty_main
HEAPSIZE     = 10240
STACKSIZE    = 24576
# UNI2H = 1

!include $(%ROOT)/mk/appsos2_pm.mk

TARGETS = $(PATH)$(PROJ1).exe subdirs
TARGET  = all

HEADERS  = $(MYDIR)sty_dlg.h $(MYDIR)sty_help.h &
           $(MYDIR)sty_main.h $(MYDIR)sty_xtrn.h

$(PATH)sty_main.obj:: $(MYDIR)sty_main.c #$(HEADERS)

$(PATH)sty_file.obj:: $(MYDIR)sty_file.c #$(HEADERS)

$(PATH)sty_edit.obj:: $(MYDIR)sty_edit.c #$(HEADERS)

$(PATH)sty_user.obj:: $(MYDIR)sty_user.c #$(HEADERS)

$(PATH)sty_init.obj:: $(MYDIR)sty_init.c #$(HEADERS)

$(PATH)sty_pnt.obj::  $(MYDIR)sty_pnt.c  #$(HEADERS)

$(PATH)sty_dlg.obj::  $(MYDIR)sty_dlg.c  #$(HEADERS)

$(PATH)sty_help.obj:: $(MYDIR)sty_help.c $(HEADERS)

$(PATH)$(PROJ1).exe: $(PATH)$(PROJ).exe $(MYDIR)$(RES).rc
 @$(SAY) RESCMP   $^.
 @$(RC) -bt=os2 -i=$(MYDIR) -i=$(%WATCOM)$(SEP)h$(SEP)os2 $]@ $[@ -fe=$^@ -fo=$(PATH)$(PROJ1).res
