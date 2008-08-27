#
# A Makefile for genext2fs
# (c) osFree project,
# valerius, 2006/10/30
#

DEST    = tools

!include $(%ROOT)/mk/all.mk

TARGETS = copy

list = $(FILESDIR) $(FILESDIR)$(SEP)tools
filelist = *.cmd *.rexx *.awk *.cfg
srcdir = $(ROOT)$(SEP)tools$(SEP)scripts
dstdir = $(FILESDIR)$(SEP)tools

copy: .SYMBOLIC
 @if exist $(FILESDIR)$(SEP)tools @%quit
 @for %i in ($(list)) do @if not exist %i $(MKDIR) %i
 #@for %i in ($(filelist)) do @$(MAKE) $(MAKEOPT) s=$(srcdir)%i d=$(dstdir)%i $(d)
 @cd $(srcdir)
 @for %i in ($(filelist)) do $(CP) %i $(DEST)
 $(CD) $(MYDIR)

#$(d): $(s)
# $(CP) $(s) $(d)
