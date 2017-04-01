#
# A Makefile for genext2fs
# (c) osFree project,
# valerius, 2006/10/30
#

PROJ = rexxtry
TRGT = $(PROJ).cmd
DESC = Rexxtry
CLEAN_ADD = *.cmd
DEST    = os2

#TARGETS = copy

#list = $(FILESDIR) $(FILESDIR)$(SEP)os2
#filelist = rexxtry.cmd
#srcdir = $(MYDIR)
#dstdir = $(FILESDIR)$(SEP)os2

#copy: .SYMBOLIC
# @for %i in ($(list)) do @if not exist %i @$(MKDIR) %i $(BLACKHOLE)
# #@for %i in ($(filelist)) do @$(MAKE) $(MAKEOPT) s=$(srcdir)%i d=$(dstdir)%i $(d)
# #@cd $(srcdir)
# @for %i in ($(filelist)) do @$(CP) %i $(DEST) $(BLACKHOLE)
# @$(CD) $(MYDIR) $(BLACKHOLE)

!include $(%ROOT)/mk/all.mk

$(PATH)$(PROJ).cmd: $(MYDIR)$(PROJ).cmd
 @$(CP) $< $^@ $(BLACKHOLE)
