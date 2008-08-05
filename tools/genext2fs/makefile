#
# A Makefile for genext2fs
# (c) osFree project,
# valerius, 2006/10/30
#

32_BITS  = 1
DEFINES  = -hw -d3 -d__WATCOM__

ADD_COPT      = $(DEFINES) -i=.

COPT_UNIX     = $(ADD_COPT_UNIX)

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

OBJS   = getline.$(O) strtof.$(O)
OBJS_1 = genext2fs.$(O)

!include $(%ROOT)/mk/all.mk

all: genext2fs.exe .SYMBOLIC

genext2fs.exe: $(OBJS) $(OBJS_1)
 @%create $^&.lnk
 @%append $^&.lnk name $^&
 @%append $^&.lnk option map=$^&.wmp
 @for %%i in ($(OBJS) $(OBJS_1)) do @%append $^&.lnk FILE %%i
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) $(LINKOPT) @$^&.lnk $(LOG)


install: .SYMBOLIC
 $(DC) ..$(SEP)bin$(SEP)genext2fs.exe $(BLACKHOLE)
 $(CP) genext2fs.exe ..$(SEP)bin $(BLACKHOLE)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
