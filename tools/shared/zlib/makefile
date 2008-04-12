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


libz.lib: $(OBJS)
 $(DC) $^@
 $(LIB) -n -fo $^@ +$(OBJS)


install: .SYMBOLIC
 $(CP) libz.lib ..$(SEP)lib

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
