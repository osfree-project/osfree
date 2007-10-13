# $Id: makefile,v 1.3 2004/08/16 04:51:12 prokushev Exp $

#ROOT=..\..\..\..

32_BITS =

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk
!include $(%ROOT)/mk/all.mk

DIR = $(ROOT)$(SEP)bin$(SEP)os2
PROJ = option2
DESC = OPTION2.EXE
OBJS = option2.obj os2cfg.obj os2nb.obj $(%ROOT)$(SEP)OS2$(SEP)CMD$(SEP)cmd$(SEP)shared$(SEP)istring.obj 
ADD_COPT = -i=..$(SEP)include -ox
#ADD_LIB = MATH387S,%watcom%$(SEP)lib286$(SEP)os2\os2
LINKOPT = LIBPATH $(LIBDIR) lib math387s, os2$(SEP)os2, os2386 


#!include $(ROOT)$(SEP)makefile.inc

all: $(PROJ).exe # $(PROJ).sym

$(PROJ).exe: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk $(LINKOPT)
 @%append $^&.lnk NAME $^@ OPTION MODNAME=$^&
 @%append $^&.lnk FORMAT OS2 LX PM
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION STACK=65536
 @%append $^&.lnk OPTION MAP=$^&.wmp
 for %i in ($(OBJS)) do @%append $^&.lnk FILE %i
 $(SAY) Linking $^@
 $(LINKER) @$^&.lnk $(LOG)
 $(RC) -bt=os2 -i=..$(SEP)include option2.rc $@ $(LOG)
 
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
