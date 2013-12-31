PROJ = pmrexxio1
PROJ1 = pmrexxio
DESC = PM REXX I/O dll
srcfiles     = $(p)pmrexxio$(e) $(p)pmrxsubs$(e)
inc          = $(MYDIR)..$(SEP)include
ADD_COPT     = -i=$(inc) -sg -3s
ADD_LINKOPT  = segment type DATA nonshared lib os2386.lib,clib3r.lib,math387r.lib,emu387.lib,rexx.lib
DLL          = 1
OPTIONS      = manyautodata, caseexact, heapsize=8192, quiet
DLLOPT       = initinstance
STACKSIZE    = 16384
EXPORTS      = RexxCreateIOWindow.1  = RexxCreateIOWindow, &
	       RexxDestroyIOWindow.2 = RexxDestroyIOWindow
DEST = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2_cmd.mk

TARGETS =  $(PATH)$(PROJ1).dll
TARGET = all

$(PATH)$(PROJ1).dll: $(PATH)$(PROJ).dll $(MYDIR)$(PROJ1).rc
 $(SAY) Compiling resource file $[.... $(LOG)
 $(RC) -bt=os2 -i=$(%WATCOM)$(SEP)h$(SEP)os2 -i=$(inc) $]@ $[@ -fe=$@ -fo=$(PATH)$(PROJ1).res
