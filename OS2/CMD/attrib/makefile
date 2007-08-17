#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

ROOT=..\..\..

!include ../../build.conf
!include ../../mk/site.mk

32_BITS  = 1

PROJ = attrib
DIR = $(ROOT)\bin\os2
DESC = Sets file attributes
#defines object file names in format objname.$(O)
OBJS = attrib.$(O)
# defines additional options for C compiler
ADD_COPT = -i=..\include

!include ../../mk/cmd.mk

#defines files mask for clean target
CLEANMASK = *.lnk *.map *.obj *.err *.log *.bak *.sym *.exe *.wmp


all: $(PROJ).exe .SYMBOLIC

$(PROJ).exe: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk SYSTEM os2v2
 @%append $^&.lnk NAME $^&
 @%append $^&.lnk $(LINKOPT)
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION MAP=$^&.wmp
 $(ADDFILES_CMD)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) @$^&.lnk $(LOG)

!ifeq UNIX FALSE

install: .SYMBOLIC
 $(CP) $(PROJ).exe ..\bin

!else

install: .SYMBOLIC
 $(CP) $(PROJ).exe ../bin

!endif

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
