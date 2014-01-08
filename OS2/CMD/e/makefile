#
# A Makefile for E
# (c) osFree project,
# author, date
#

PROJ        = e
DESC        = osFree System Editor
#defines object file names in format objname.$(O)
srcfiles    = $(p)sty_main$(e) $(p)sty_user$(e) $(p)sty_init$(e) &
              $(p)sty_pnt$(e)  $(p)sty_dlg$(e)  $(p)sty_file$(e) &
              $(p)sty_edit$(e) $(p)sty_help$(e)
# defines additional options for C compiler
#ADD_COPT    = -sg -3s -ecc -i=$(MYDIR) # until UniAPI modules will be done
ADD_COPT    = -bm -od -mf -sg -d2 -mf -i=$(MYDIR)
#ADD_LINKOPT = lib os2386.lib, clib3s.lib
#STUB       = $(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe
WINDOWCOMPAT = 1
#NOLIBS = 1
RESOURCE    = $(PATH)sty_main.res
RCOPT       += -i=$(%WATCOM)$(SEP)h$(SEP)os2 # until UniAPI modules will be done
INSTALL_ADD = 1
DEST1       = os2$(SEP)help
OPTIONS      = HEAPSIZE=10240
STACKSIZE   = 24576
# UNI2H = 1

SUF         = .bmp .ico

!include $(%ROOT)/mk/appsos2_pm.mk

TARGETS = $(PATH)$(PROJ).exe install_add

install_add: $(PATH)$(PROJ).hlp
 $(MDHIER) $(DEST1)
 @if exist $< $(CP) $< $(DEST1)

$(PATH)$(PROJ).hlp: $(MYDIR)e.ipf

.bmp: $(MYDIR)

.ico: $(MYDIR)

HEADERS  = $(MYDIR)sty_dlg.h $(MYDIR)sty_help.h &
           $(MYDIR)sty_main.h $(MYDIR)sty_xtrn.h

$(PATH)sty_main.obj: $(MYDIR)sty_main.c #$(HEADERS)

$(PATH)sty_file.obj: $(MYDIR)sty_file.c #$(HEADERS)

$(PATH)sty_edit.obj: $(MYDIR)sty_edit.c #$(HEADERS)

$(PATH)sty_user.obj: $(MYDIR)sty_user.c #$(HEADERS)

$(PATH)sty_init.obj: $(MYDIR)sty_init.c #$(HEADERS)

$(PATH)sty_pnt.obj:  $(MYDIR)sty_pnt.c  #$(HEADERS)

$(PATH)sty_dlg.obj:  $(MYDIR)sty_dlg.c  #$(HEADERS)

$(PATH)sty_help.obj: $(MYDIR)sty_help.c #$(HEADERS)
