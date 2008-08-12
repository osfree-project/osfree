#
# A Makefile for genext2fs
# (c) osFree project,
# valerius, 2006/10/30
#

DEST    = tools

!include $(%ROOT)/mk/all.mk

TARGETS = copy

list = $(FILESDIR) $(FILESDIR)$(SEP)tools
filelist = *.cmd *.rexx *.the *.awk *.cfg

copy: .SYMBOLIC
 @if exist $(FILESDIR)$(SEP)tools @%quit
 @for %i in ($(list)) do @if not exist %i $(MKDIR) %i
 @cd $(ROOT)$(SEP)tools$(SEP)scripts
 @for %i in ($(filelist)) do $(CP) %i $(DEST)
 $(CD) $(MYDIR)
