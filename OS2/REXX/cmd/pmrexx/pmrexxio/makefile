# PROJ = pmrexxio
PROJ = pmrexxio1
PROJ1 = pmrexxio
TRGT = $(PROJ1).dll
# TRGT = $(PROJ).dll
DESC = PM REXX I/O dll
srcfiles     = $(p)pmrexxio$(e) $(p)pmrxsubs$(e)
inc          = $(MYDIR)..$(SEP)include -i=$(%WATCOM)$(SEP)h$(SEP)os2 -i=$(BLD)include -i=$(BLD)include$(SEP)os2 -i=$(BLD)include$(SEP)shared
ADD_COPT     = -i=$(inc) -sg -3s # -3r
ADD_RCOPT    = -i=$(inc)
ADD_LINKOPT  = segment type DATA nonshared lib os2386.lib,clib3s.lib,math387s.lib,emu387.lib,rexx.lib
DLL          = 1
WINDOWCOMPAT = 1
OPTIONS      = manyautodata, caseexact, heapsize=8192, quiet
DLLOPT       = initinstance
STACKSIZE    = 16384
EXPORTS      = RexxCreateIOWindow.1  = RexxCreateIOWindow, &
	       RexxDestroyIOWindow.2 = RexxDestroyIOWindow
# RESOURCE     = $(PATH)$(PROJ).res
DEST         = os2$(SEP)dll
UNI2H        = 1
C            = s

!include $(%ROOT)tools/mk/appsos2.mk

#TARGETS =  $(PATH)$(PROJ).dll
TARGETS =  $(PATH)$(PROJ1).dll
#TARGET = all

$(PATH)$(PROJ1).dll: $(PATH)$(PROJ).dll $(MYDIR)$(PROJ1).rc
 @$(SAY) RESCMP   $^. $(LOG)
 $(RC) -bt=os2 -i=$(inc) $]@ $[@ -fe=$@ -fo=$(PATH)$(PROJ1).res
