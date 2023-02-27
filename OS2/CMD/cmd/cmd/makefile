
PROJ = cmd1
PROJ1 = cmd
#TRGT = $(PROJ1).exe
DESC = Command processor
srcfiles = $(p)main$(e) $(p)parser$(e) $(p)os2init$(e) $(p)cmds$(e) $(p)expand$(e) $(p)eval$(e) &
           $(p)ea$(e) $(p)dircmds$(e) $(p)filecmds$(e) $(p)syscmds$(e) $(p)os2cmds$(e)   &
           $(p)os2calls$(e) $(p)misc$(e) $(p)env$(e) $(p)batch$(e) $(p)screenio$(e) $(p)msg$(e) &
           $(p)error$(e) $(p)window$(e) $(p)listall$(e) $(p)lines$(e) $(p)kparse$(e)        &
           $(p)iofmt32$(e) $(p)select$(e) $(p)os2tty$(e) $(p)iparse$(e) $(p)dbcs$(e)   &
           $(p)bcd32$(e) $(p)os32util$(e) $(p)..$(SEP)shared$(SEP)istring$(e) $(p)wrappers$(e)

ADD_LINKOPT = lib os2386.lib, clib3r.lib, math387r.lib, emu387.lib #, sub32.lib
# ADD_LINKOPT =
STUB        = $(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe
# RESOURCE    = $(PATH)4os2.res
IMPORTS     = DOSGETINFOSEG               DOSCALLS.8

!include $(%ROOT)tools/mk/appsos2_cmd.mk
!include $(MYDIR)..$(SEP)port$(SEP)4os2.mk

ADD_COPT    = -i=$(%ROOT)include$(SEP)os3 -i=$(BLD)include -ox
NODEFAULTLIBS = 1
#    $(BLD)$(SEP)lib$(SEP)os2386.lib,
#   clibext.lib,
inc         =  -i=. -i=$(PORT_BASE)h -i=$(MYDIR)..$(SEP)..$(SEP)include &
               -i=$(BLD)include # -i=$(BLD)include$(SEP)os2 -i=$(BLD)include$(SEP)shared
ADD_COPT    += $(inc)
#UNI2H = 1
#DEST = os2

.asm: $(PORT_BASE)a

.c: $(PORT_BASE)c

.h: $(PORT_BASE)h

TARGETS =  $(PATH)$(PROJ1).exe

$(PATH)$(PROJ1).exe: $(PATH)$(PROJ).exe $(PORT_BASE)4os2.rc
 @$(SAY) RESCMP   $^. $(LOG)
 $(RC) -bt=os2 -i=$(inc) $]@ $[@ -fe=$@ -fo=$(PATH)4os2.res
