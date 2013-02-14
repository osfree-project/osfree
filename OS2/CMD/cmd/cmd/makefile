
PROJ = cmd
DESC = Command processor
srcfiles = $(p)main$(e) $(p)parser$(e) $(p)os2init$(e) $(p)cmds$(e) $(p)expand$(e) $(p)eval$(e) &
           $(p)ea$(e) $(p)dircmds$(e) $(p)filecmds$(e) $(p)syscmds$(e) $(p)os2cmds$(e)   &
           $(p)os2calls$(e) $(p)misc$(e) $(p)env$(e) $(p)batch$(e) $(p)screenio$(e) $(p)msg$(e) &
           $(p)error$(e) $(p)window$(e) $(p)listall$(e) $(p)lines$(e) $(p)kparse$(e)        &
           $(p)iofmt32$(e) $(p)select$(e) $(p)os2tty$(e) $(p)iparse$(e)     &
           $(p)bcd32$(e) $(p)os32util$(e) $(p)..$(SEP)shared$(SEP)istring$(e)
ADD_COPT    = -i=$(MYDIR)..$(SEP)include -i=$(MYDIR)..$(SEP)..$(SEP)include -ox
NODEFAULTLIBS = 1
#    $(BLD)$(SEP)lib$(SEP)os2386.lib,
#   clibext.lib,
ADD_LINKOPT = lib os2386.lib,  clib3r.lib, math387r.lib, emu387.lib, sub32.lib
#ADD_LINKOPT =
#STUB        = $(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)$(PROJ).exe
RESOURCE    = $(PATH)4os2.res
IMPORTS     = DOSGETINFOSEG               DOSCALLS.8

!include $(%ROOT)/mk/appsos2.mk
