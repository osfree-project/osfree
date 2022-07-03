PROJ = pmrexx1
PROJ1 = pmrexx
TRGT = $(PROJ1).exe
DESC = PM REXX console 
DIRS = pmrexxio help
srcfiles     = $(p)pmrexx$(e) $(p)rxhafile$(e)
inc          = $(MYDIR)include
ADD_COPT     = -i=$(inc) -sg -3s # -3r
ADD_LINKOPT  = lib os2386.lib,clib3s.lib,math387s.lib,emu387.lib,rexx.lib
OPTIONS      = heapsize=8192
STACKSIZE    = 16384
PM           = 1
IMPORTS      = RexxCreateIOWindow    PMREXXIO.RexxCreateIOWindow, &
    	       RexxDestroyIOWindow   PMREXXIO.RexxDestroyIOWindow
UNI2H        = 1
C            = s

!include $(%ROOT)/mk/appsos2.mk

TARGETS = $(PATH)$(PROJ1).exe # subdirs
#TARGET = all

$(PATH)$(PROJ1).exe: $(PATH)$(PROJ).exe $(MYDIR)$(PROJ1).rc
 @$(SAY) RESCMP   $^. $(LOG)
 $(RC) -bt=os2 -i=$(%WATCOM)$(SEP)h$(SEP)os2 -i=$(inc) $]@ $[@ -fe=$@ -fo=$(PATH)$(PROJ1).res
