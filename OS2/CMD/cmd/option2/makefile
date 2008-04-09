# $Id: makefile,v 1.3 2004/08/16 04:51:12 prokushev Exp $


!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS = 1
DIR = $(ROOT)$(SEP)bin$(SEP)os2
PROJ = option2
DESC = OPTION2.EXE
OBJS = option2.obj os2cfg.obj os2nb.obj $(%ROOT)$(SEP)OS2$(SEP)CMD$(SEP)cmd$(SEP)shared$(SEP)istring.obj
ADD_COPT = -i=..$(SEP)include -ox

!include $(%ROOT)/mk/all.mk


all: $(PROJ).exe # $(PROJ).sym

$(PROJ).exe: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk SYSTEM os2v2
 @%append $^&.lnk NAME $^&
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION MAP=$^&.wmp
 $(ADDFILES_CMD)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) $(LINKOPT) @$^&.lnk $(LOG)
 $(RC) $(RCOPT) -i=..$(SEP)include option2.rc $@ $(LOG)

#@%append $^&.lnk LIBRARY $(LINKLIB)

# Clean target
clean : .symbolic
    $(DC) *.obj *.res *.map *.wmp *.lnk  $(LOG)

# This part "if exist" does not work with OW in GNU/Linux.
#@if exist *.obj $(DC)  *.obj $(LOG)
#@if exist *.res $(DC)  *.res $(LOG)
#@if exist *.map $(DC)  *.map $(LOG)
#@if exist *.wmp $(DC)  *.wmp $(LOG)
#@if exist *.lnk $(DC)  *.lnk $(LOG)
