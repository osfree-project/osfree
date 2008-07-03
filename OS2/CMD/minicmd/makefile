#
# A Makefile for MiniCMD
# (c) osFree project,
# prokushev, 2008
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1

PROJ = minicmd
DIR = $(ROOT)$(SEP)bin$(SEP)os2
DESC = Minimalistic Command line processor
#defines object file names in format objname.$(O)
OBJS = minicmd.$(O) commands.$(O)

!include $(%ROOT)/mk/all.mk


all: $(PROJ).exe .SYMBOLIC

$(PROJ).exe: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk SYSTEM os2v2
 @%append $^&.lnk NAME $^&
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk OPTION STUB=$(FILESDIR)$(SEP)dos$(SEP)stub.exe
 $(ADDFILES_CMD)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) $(LINKOPT) @$^&.lnk $(LOG)
 @$(MAKE) $(MAKEOPT) install

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)

install: .SYMBOLIC
 $(SAY) Making install... $(LOG)
 $(CP) $(PROJ).exe $(FILESDIR)$(SEP)os2
