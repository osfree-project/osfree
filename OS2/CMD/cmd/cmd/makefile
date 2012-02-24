
PROJ = cmd
DESC = Command processor
srcfiles = $(p)main$(e) $(p)parser$(e) $(p)os2init$(e) $(p)cmds$(e) $(p)expand$(e) $(p)eval$(e) &
           $(p)ea$(e) $(p)dircmds$(e) $(p)filecmds$(e) $(p)syscmds$(e) $(p)os2cmds$(e)   &
           $(p)os2calls$(e) $(p)misc$(e) $(p)env$(e) $(p)batch$(e) $(p)screenio$(e) $(p)msg$(e) &
           $(p)error$(e) $(p)window$(e) $(p)listall$(e) $(p)lines$(e) $(p)kparse$(e)        &
           $(p)iofmt32$(e) $(p)select$(e) $(p)os2tty$(e) $(p)iparse$(e)     &
           $(p)bcd32$(e) $(p)os32util$(e) $(p)..$(SEP)shared$(SEP)istring$(e)
ADD_COPT    = -i=$(MYDIR)..$(SEP)include -i=$(MYDIR)..$(SEP)..$(SEP)include -ox
ADD_LINKOPT = lib math387s, os2$(SEP)os2, os2386
#STUB        = $(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)$(PROJ).exe
RESOURCE    = $(PATH)4os2.res
IMPORTS     = KbdStringIn                 EMXWRAP.209, &
              KbdCharIn                   EMXWRAP.204, &
              KbdPeek                     EMXWRAP.222, &
	      VioReadcellStr              EMXWRAP.124, &
	      VioWrtNAttr                 EMXWRAP.126, &
	      VioSetCurType		  EMXWRAP.132, &
	      VioWrtNCell		  EMXWRAP.152, &
	      VioScrollUp                 EMXWRAP.107, &
	      VioScrollDn                 EMXWRAP.147, &
	      VioGetCurPos		  EMXWRAP.109, &
	      VioSetCurPos		  EMXWRAP.115, &
	      VioWrtCellStr		  EMXWRAP.110, &
	      VioGetAnsi                  EMXWRAP.103, &
	      VioSetAnsi                  EMXWRAP.105, &
	      VioGetMode	          EMXWRAP.121, &
	      VioGetCp		 	  EMXWRAP.140, &
	      VioSetCp		 	  EMXWRAP.142, &
	      VioWrtTTY                   EMXWRAP.119, &
	      VioGetConfig		  EMXWRAP.146, &
	      VioReadCellStr		  EMXWRAP.124, &
	      VioWrtCharStrAtt		  EMXWRAP.148, &
	      VioSetState		  EMXWRAP.151, &
              DOSGETINFOSEG               DOSCALLS.8

!include $(%ROOT)/mk/appsos2.mk