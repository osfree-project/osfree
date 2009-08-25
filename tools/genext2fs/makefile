#
# A Makefile for genext2fs
# (c) osFree project,
# valerius, 2006/10/30
#

32_BITS  = 1
PROJ = genext2fs
DESC = Generate ext2fs filesystem image

DEFINES  = -hw -d3 -d__WATCOM__
ADD_COPT      = $(DEFINES) -i=.

srcfiles = $(p)getline$(e) $(p)strtof$(e) $(p)genext2fs$(e)

!include $(%ROOT)/mk/tools.mk
