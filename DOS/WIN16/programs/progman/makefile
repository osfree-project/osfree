#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

# use 16-bit compiler
#32_BITS  = 1

PROJ = progman
DESC = Windows Program Manager
#defines object file names in format objname.$(O)
OBJS = dialog.$(O) group.$(O) grpfile.$(O) main.$(O) program.$(O) string.$(O)

# defines additional options for C compiler
ADD_COPT = -zW -sg -zw -bt=windows -bw -bg -d3 -db -hw

!include $(%ROOT)/mk/all.mk


all: $(PROJ).exe $(PROJ).sym .SYMBOLIC

$(PROJ).exe: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk SYSTEM windows
 @%append $^&.lnk NAME $^&
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk debug watcom
 @%append $^&.lnk import
 @%append $^&.lnk         SHELLABOUT SHELL.22,
 @%append $^&.lnk         GETOPENFILENAME COMMDLG.1,
 @%append $^&.lnk         GETCURRENTDIRECTORY WINSMSG.20,
 @%append $^&.lnk         EXTRACTICON     SHELL.34
 $(ADDFILES_CMD)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) $(LINKOPT) @$^&.lnk $(LOG)
 wrc -bt=windows rsrc.rc $(PROJ).exe
 $(MAKE) install

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)

install: .SYMBOLIC
 $(SAY) Making install... $(LOG)
 $(CP) $(PROJ).exe $(FILESDIR)$(SEP)dos
