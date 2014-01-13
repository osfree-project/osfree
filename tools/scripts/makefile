#
# A Makefile for genext2fs
# (c) osFree project,
# valerius, 2006/10/30
#

DEST    = tools

!include $(%ROOT)/mk/all.mk

TARGETS = tools

list = $(FILESDIR) $(FILESDIR)$(SEP)tools
filelist = *.cmd *.awk *.cfg
srcdir = $(%ROOT)$(SEP)tools$(SEP)scripts

tools: copy  .symbolic
 @cd .. 
 @$(MAKE) $(MAKEOPTS) subdirs
 $(CD) $(MYDIR)

copy: .symbolic
 @if exist $(FILESDIR)$(SEP)base-copied-flag @%quit
 @for %i in ($(list)) do @if not exist %i $(MKDIR) %i
 @cd $(srcdir)
 @for %i in ($(filelist)) do $(CP) %i $(DEST)
 @touch $(FILESDIR)$(SEP)base-copied-flag

 