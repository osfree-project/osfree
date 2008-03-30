#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1

PROJ = PMWindow
DIR = $(ROOT)\bin\os2
DESC = Control utility for ANSI
#defines object file names in format objname.$(O)
OBJS = fs_pmdev.$(O)
# defines additional options for C compiler
ADD_COPT = -i=..\include -bd
ADD_LIB = cmd_shared.lib,all_shared.lib
ADD_LINKOPT=lib os2386.lib lib clib3r.lib
!include $(%ROOT)/mk/all.mk

#defines files mask for clean target

all: $(PROJ).dll .SYMBOLIC

$(PROJ).dll: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk SYSTEM os2v2 dll
 @%append $^&.lnk NAME $^&
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk export FPM_DeviceStart=FPM_PMWinStart
 $(ADDFILES_CMD)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) $(LINKOPT) @$^&.lnk $(LOG)


.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)

install: .SYMBOLIC
 $(SAY) Making install... $(LOG)
 $(CP) $(PROJ).dll $(FILESDIR)$(SEP)os2$(SEP)dll
 $(MAKE) $(MAKEOPT) TARGET=$^@ subdirs
