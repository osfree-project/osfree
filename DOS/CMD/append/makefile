#
# A Makefile for APPEND
# (c) osFree project,
# author, date
#

PROJ      = append
TRGT      = $(PROJ).com
DESC      = Append dirs to search path
srcfiles  = $(p)segs$(e) $(p)environ$(e) $(p)cmdline$(e) $(p)int2f$(e) $(p)int21$(e) $(p)append$(e)
COM       = 1

!include $(%ROOT)/mk/appsdos.mk

$(p)append$(e): append.asm append.inc cmdline.inc useful.inc

$(p)cmdline$(e): cmdline.asm append.inc cmdline.inc useful.inc

$(p)int2f$(e): int2f.asm append.inc cmdline.inc useful.inc

$(p)int21$(e): int21.asm append.inc useful.inc
