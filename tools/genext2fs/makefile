#
# A Makefile for genext2fs
# (c) osFree project,
# valerius, 2006/10/30
#

32_BITS  = 1
DEFINES  = -hw -d3 -d__OS2__ -d__WATCOM__
DEFINES_UNIX = -hw -d3 -d__LINUX__ -d__WATCOM__

ADD_COPT      = $(DEFINES) -i=.
ADD_COPT_UNIX = $(DEFINES_UNIX) -i=.

COPT_UNIX     = $(ADD_COPT_UNIX)

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

OBJS   = getline.$(O) strtof.$(O)
OBJS_1 = genext2fs.$(O)
OBJS_2 = genext2fs_u.$(O)

!include $(%ROOT)/mk/all.mk

all: os2 linux .SYMBOLIC

os2: genext2fs.exe .SYMBOLIC

linux: genext2fs .SYMBOLIC

genext2fs.exe: $(OBJS) $(OBJS_1)
 @%create $^&.lnk
 @%append $^&.lnk system os2v2
 @%append $^&.lnk name $^&
 @%append $^&.lnk option map=$^&.wmp
 @for %%i in ($(OBJS) $(OBJS_1)) do @%append $^&.lnk FILE %%i
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) @$^&.lnk $(LOG)

genext2fs: $(OBJS) $(OBJS_2)
 @%create $^&_u.lnk
 @%append $^&_u.lnk system linux
 @%append $^&_u.lnk name $^&
 @%append $^&_u.lnk option map=$^&_u.map
 @for %%i in ($(OBJS) $(OBJS_2)) do @%append $^&_u.lnk FILE %%i
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) $(LINKOPT) @$^&_u.lnk $(LOG)

install: .SYMBOLIC
 $(DC) ..$(SEP)bin$(SEP)genext2fs.exe $(BLACKHOLE)
 $(DC) ..$(SEP)bin$(SEP)genext2fs $(BLACKHOLE)
 $(CP) genext2fs.exe ..$(SEP)bin $(BLACKHOLE)
 $(RN) genext2fs.elf genext2fs $(BLACKHOLE)
 $(CP) genext2fs ..$(SEP)bin $(BLACKHOLE)

genext2fs_u.$(O): genext2fs.c
 $(SAY) Compiling $< $(LOG)
 $(CC) $(COPT_UNIX) -fo=$^&.$(O) $< $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
