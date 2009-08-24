#
# A main Makefile for OS/3 boot sequence project
# (c) osFree project,
# valerius, 2006/10/30
#

#srcfiles     = $(p)drivemap$(e) $(p)end$(e) $(p)modesw$(e) $(p)cfgparse$(e) $(p)cmdline$(e) &
#               $(p)setdev$(e) $(p)wrap$(e)

DIRS         = shared filesys term preldr loader kernels
DEST         = boot$(SEP)loader

!include $(%ROOT)/mk/loader.mk

#p            = $(PATH)
#e            = .$$(O)
#OBJS         = $+$(srcs)$-

TARGETS      = $(MYDIR)boot.cfg $(MYDIR)preldr.ini subdirs
TARGET       = all
