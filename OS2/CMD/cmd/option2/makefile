PROJ  = option1
PROJ1 = option2
#TRGT = $(PROJ1).exe
DESC =  Settings notebook for 4os2
srcfiles    = $(p)option2$(e) $(p)os2cfg$(e) $(p)os2nb$(e) $(p)..$(SEP)shared$(SEP)istring$(e)
#inc         =  -i=. -i=$(MYDIR)..$(SEP)include -i=$(MYDIR)..$(SEP)..$(SEP)include &
#               -i=$(BLD)include$(SEP)shared -i=$(BLD)include -i=$(BLD)include$(SEP)os2
STUB        = $(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe
# RESOURCE    = $(PATH)$(PROJ).res
#UNI2H = 1
WINDOWCOMPAT = 1
DEST = os2

!include $(%ROOT)tools/mk/appsos2_pm.mk
!include $(MYDIR)..$(SEP)port$(SEP)4os2.mk

inc         =  -i=. -i=$(PORT_BASE)h -i=$(MYDIR)..$(SEP)..$(SEP)include &
               -i=$(%WATCOM)$(SEP)h$(SEP)os2 -i=$(BLD)include
ADD_COPT    += $(inc)

.c: $(PORT_BASE)c

.h: $(PORT_BASE)h

TARGETS = $(PATH)$(PROJ1).exe
#TARGET = all

$(PATH)$(PROJ1).exe: $(PORT_BASE)$(PROJ1).rc $(PATH)$(PROJ).exe
 @$(SAY) RESCMP   $^. $(LOG)
 @$(RC) -bt=os2 $(inc) $[@ $]@ -fo=$(PATH)$(PROJ1).res -fe=$@
