#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#


!include ../../../build.conf
!include ../../../mk/site.mk

ROOT=..$(SEP)..$(SEP)..
32_BITS  = 1

PROJ = chkdsk
DIR = $(ROOT)$(SEP)bin$(SEP)os2
DESC = CHKDSK front-end
#defines object file names in format objname.$(O)
OBJS = chkdsk.$(O)
# defines additional options for C compiler
ADD_COPT = -i=..$(SEP)include
ADD_LIB = ,cmd_shared.lib,all_shared.lib

!include ../../../mk/cmd.mk

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
 $(MAKE) install

install: .SYMBOLIC
 $(CP) $(PROJ).exe $(DIR)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
