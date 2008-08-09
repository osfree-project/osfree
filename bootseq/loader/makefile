#
# A main Makefile for OS/3 boot sequence project
# (c) osFree project,
# valerius, 2006/10/30
#

srcs         = $(p)drivemap$(e) $(p)end$(e) $(p)modesw$(e) $(p)cfgparse$(e) $(p)cmdline$(e) &
               $(p)setdev$(e) $(p)wrap$(e)

DIRS         = minilibc filesys term # preldr loader kernels
DIR          = loader
RELDIR       = bootseq$(SEP)$(DIR)$(SEP)
MYDIR        = $(%ROOT)$(SEP)$(RELDIR)
PATH         = $(RELDIR)

!include $(%ROOT)/mk/loader.mk

p            = $(PATH)
e            = .$$(O)
OBJS         = $+$(srcs)$-

TARGETS      = $(OBJS) subdirs
