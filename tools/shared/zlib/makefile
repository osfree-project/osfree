#
# A Makefile for Zlib
# (c) osFree project,
# valerius, 2006/10/30
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1
DIR = $(%ROOT)$(SEP)tools$(SEP)lib
ADD_COPT = -d__WATCOM__ -d__OS2__ -i=. -i=..$(SEP)..$(SEP)include

OBJS = adler32.$(O) compress.$(O) crc32.$(O) gzio.$(O) &
       uncompr.$(O) deflate.$(O) trees.$(O) zutil.$(O) &
       inflate.$(O) infblock.$(O) inftrees.$(O) infcodes.$(O) &
       infutil.$(O) inffast.$(O)

!include $(%ROOT)/mk/all.mk

CLEANMASK = *.lnk *.wmp *.obj *.err *.log *.bak *.sym

all: libz.lib .SYMBOLIC
 $(CP) libz.lib $(DIR)$(SEP) $(BLACKHOLE)

sub: example.exe minigzip.exe .SYMBOLIC

libz.lib: $(OBJS)
 $(DC) $^@
 $(LIB) -n -fo $^@ +$(OBJS)

example.exe: example.$(O) libz.lib
 @%create $^&.lnk
 @%append $^&.lnk SYSTEM os2v2
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk NAME $^&
 @%append $^&.lnk LIBRARY libz
 @%append $^&.lnk LIBRARY clib3r
 @%append $^&.lnk FILE example.$(O)
 $(ADDFILES_CMD)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) $(LINKOPT) @$^&.lnk $(LOG)

minigzip.exe: minigzip.$(O) libz.lib
 @%create $^&.lnk
 @%append $^&.lnk SYSTEM os2v2
 @%append $^&.lnk OPTION MAP=$^&.map
 @%append $^&.lnk NAME $^&
 @%append $^&.lnk LIBRARY libz
 @%append $^&.lnk LIBRARY clib3r
 @%append $^&.lnk FILE minigzip.$(O)
 $(ADDFILES_CMD)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) $(LINKOPT) @$^&.lnk $(LOG)

install: .SYMBOLIC
 $(CP) libz.lib ..$(SEP)lib

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
