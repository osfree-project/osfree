# $Id: makefile,v 1.3 2004/08/16 04:51:12 prokushev Exp $

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS = 1
DIR = $(%ROOT)$(SEP)bin$(SEP)os2
PROJ = keystack
DESC = KEYSTACK.EXE
OBJS = keystack.obj
ADD_COPT = -i=..$(SEP)include -ox

!include $(%ROOT)/mk/all.mk

all: $(PROJ).exe #$(PROJ).sym

$(PROJ).exe: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk SYSTEM os2v2
 @%append $^&.lnk NAME $^&
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION MAP=$^&.wmp
 $(ADDFILES_CMD)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) $(LINKOPT) @$^&.lnk $(LOG)


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
