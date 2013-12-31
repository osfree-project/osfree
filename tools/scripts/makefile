#
# A Makefile for genext2fs
# (c) osFree project,
# valerius, 2006/10/30
#

DEST    = tools

!include $(%ROOT)/mk/all.mk

TARGETS = copy

list = $(FILESDIR) $(FILESDIR)$(SEP)tools
filelist = *.cmd *.awk *.cfg
srcdir = $(%ROOT)$(SEP)tools$(SEP)scripts

copy: .symbolic
 @if exist $(FILESDIR)$(SEP)base-built-flag @%quit
 @for %i in ($(list)) do @if not exist %i $(MKDIR) %i
 @cd $(srcdir)
 @for %i in ($(filelist)) do $(CP) %i $(DEST)
 @touch $(FILESDIR)$(SEP)base-built-flag
 @$(CD) .. && $(MAKE) $(MAKEOPTS)
 $(CD) $(MYDIR)
