#
# A Makefile for OS/2 Server
# (c) osFree project,
# author, date
#

PROJ = os2server
DESC = OS/2 Personality Server
srcfiles = &
$(p)shared$(SEP)os2server$(SEP)main$(e) &
$(p)shared$(SEP)modmgr$(SEP)modmgr$(e) &
$(p)shared$(SEP)ixfmgr$(SEP)ixfmgr$(e) &
$(p)shared$(SEP)ixfmgr$(SEP)lx$(SEP)lx$(e) &
$(p)shared$(SEP)ixfmgr$(SEP)ne$(SEP)ne$(e) &
$(p)shared$(SEP)ixfmgr$(SEP)lx$(SEP)fixuplx$(e) &
$(p)shared$(SEP)ixfmgr$(SEP)lx$(SEP)loadobjlx$(e) &
$(p)shared$(SEP)ixfmgr$(SEP)lx$(SEP)modlx$(e) &
$(p)shared$(SEP)ixfmgr$(SEP)lx$(SEP)execlx$(e) &
$(p)shared$(SEP)ixfmgr$(SEP)lx$(SEP)debuglx$(e) &
$(p)shared$(SEP)memmgr$(SEP)memmgr$(e) &
$(p)shared$(SEP)cfgparser$(SEP)cfgparser$(e) &
$(p)shared$(SEP)processmgr$(SEP)processlx$(e) &
$(p)os2$(SEP)io$(e) &
$(p)os2$(SEP)native_dynlink$(e)
# defines additional options for C compiler
ADD_COPT    = -i=$(MYDIR)include
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe

!include $(%ROOT)/mk/appsos2.mk
