#
# A Makefile for OS/3 boot sequence project
# micro executable format drivers
# (c) osFree project,
# valerius, 2006/10/30
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

FMT        = bin
DIR        = bootseq$(SEP)uXFD$(SEP)$(FMT)
MYDIR      = $(%ROOT)$(SEP)$(DIR)

DIRS       =
ADD_COPT   = -od -s $(DEFINES) -i=. -i=.. -i=$(%ROOT)$(SEP)include
ADD_ASMOPT =

PATH       = $(MYDIR)$(PS)$(MYDIR)$(SEP)..

STARTUP    =
SPEC_OBJS  = load.$(O)
GEN_OBJS   = init.$(O)

OBJS       = $(STARTUP) $(GEN_OBJS) $(SPEC_OBJS)

!include $(%ROOT)/mk/bootseq.mk

all: bin.xfd
 $(MAKE) TARGET=$^@ subdirs

bin.xfd: bin.com
 $(SAY) Making $^@ ... $(LOG)
 @$(DC) $^@ $(LOG)
 @$(RN) $< $^@ $(LOG)
 #$(MKBIN) $< $^@ $(LOG)

bin.com: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk SYSTEM dos com
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk OPTION NODEFAULTLIBS
 @%append $^&.lnk NAME $^&
 @%append $^&.lnk ALIAS _l=LIPPtr
 @%append $^&.lnk ALIAS fmt_load=_fmt_load
 @%append $^&.lnk LIBRARY $(LIBC)
 @%append $^&.lnk PATH $(PATH)
 $(ADDFILES_CMD)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) @$^&.lnk $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
 $(MAKE) TARGET=$^@ subdirs
