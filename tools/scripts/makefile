#
# A Makefile for genext2fs
# (c) osFree project,
# valerius, 2006/10/30
#

DEST    = host$(SEP)$(%HOST)$(SEP)bin

#TARGETS = tools

!include $(%ROOT)/mk/all.mk

list = $(FILESDIR) $(FILESDIR)$(SEP)host$(SEP)$(%HOST)$(SEP)bin
filelist = *.cmd *.awk *.cfg
srcdir = $(ROOT)tools$(SEP)scripts

tools: copy subdirs .symbolic
 @cd .. 
 @$(MAKE) $(MAKEOPT) subdirs
 @$(CD) $(MYDIR)

copy: .symbolic
 @if exist $(FILESDIR)$(SEP)host$(SEP)$(%HOST)$(SEP)base-copied-flag.flg @%quit
 @for %i in ($(list)) do @if not exist %i @$(REXX) $(ROOT)tools$(SEP)scripts$(SEP)mdhier.cmd %i
 @cd $(srcdir)
 @for %i in ($(filelist)) do @$(CP) %i $(DEST) $(BLACKHOLE)
 @wtouch $(FILESDIR)$(SEP)host$(SEP)$(%HOST)$(SEP)base-copied-flag.flg

