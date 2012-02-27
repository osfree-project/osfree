#
# A Makefile for genext2fs
# (c) osFree project,
# valerius, 2006/10/30
#

DEST    = os2

!include $(%ROOT)/mk/appsos2.mk

TARGETS = copy

list = $(FILESDIR) $(FILESDIR)$(SEP)os2
filelist = *.cmd
srcdir - $(MYDIR)
dstdir = $(FILESDIR)$(SEP)os2

copy: .SYMBOLIC
 @for %i in ($(list)) do @if not exist %i $(MKDIR) %i
 #@for %i in ($(filelist)) do $(MAKE) $(MAKEOPT) s=$(srcdir)%i d=$(dstdir)%i $(d)
 @cd $(srcdir)
 @for %i in ($(filelist)) do $(CP) %i $(DEST)
 $(CD) $(MYDIR)

