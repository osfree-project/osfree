#
# A Makefile for genext2fs
# (c) osFree project,
# valerius, 2006/10/30
#

DEST    = tools

#TARGETS = tools

!include $(%ROOT)/mk/all.mk

list = $(FILESDIR) $(FILESDIR)$(SEP)tools
filelist = *.cmd *.awk *.cfg
srcdir = $(%ROOT)$(SEP)tools$(SEP)scripts

tools: copy  .symbolic
 @cd .. 
 @$(MAKE) $(MAKEOPT) subdirs
 @$(CD) $(MYDIR)

copy: .symbolic
 @if exist $(FILESDIR)$(SEP)base-copied-flag.flg @%quit
 @for %i in ($(list)) do @if not exist %i @$(MKDIR) %i
 @cd $(srcdir)
 @for %i in ($(filelist)) do @$(CP) %i $(DEST) $(BLACKHOLE)
 @wtouch $(FILESDIR)$(SEP)base-copied-flag.flg

 