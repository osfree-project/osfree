# $Id: makefile,v 1.3 2004/08/16 04:51:12 prokushev Exp $

#ROOT=..$(SEP)..$(SEP)..$(SEP)..

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk
!include $(%ROOT)/mk/all.mk

DIR = $(%ROOT)$(SEP)bin$(SEP)os2
PROJ = keystack
DESC = KEYSTACK.EXE
OBJS = keystack.obj
ADD_COPT = -i=..$(SEP)include -ox
# ADD_COPT = -i=..$(SEP)include -ox
#ADD_LIB = MATH387S,os2
#ADD_LIB = lib math387s, os2$(SEP)os2, os2386
LINKOPT = lib math387s, os2$(SEP)os2, os2386 

#!include $(%ROOT)$(SEP)makefile.inc

all: $(PROJ).exe #$(PROJ).sym

# $(DIR)\$(PROJ).exe: $(OBJS) 
$(PROJ).exe: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk $(LINKOPT)
 @%append $^&.lnk NAME $^@ OPTION MODNAME=$^&
 @%append $^&.lnk FORMAT OS2 LX PMCOMPATIBLE
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION STACK=65408
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @for %i in ($(OBJS)) do @%append $^&.lnk FILE %i
 $(SAY) Linking $^@
 @$(LINKER) @$^&.lnk $(LOG)

# Some strange error message about a to big stack segment came up, so I lowered it 
# with 128 bytes.
#@%append $^&.lnk OPTION STACK=65536 
#@%append $^&.lnk $(ADD_LIB)
#@%append $^&.lnk LIBRARY $(ADD_LIB)

# Clean target
clean : .symbolic
    @if exist *.obj @del /q *.obj $(LOG)
    @if exist *.map @del /q *.map $(LOG)
    @if exist *.wmp @del /q *.wmp $(LOG)
    @if exist *.lnk @del /q *.lnk $(LOG)
