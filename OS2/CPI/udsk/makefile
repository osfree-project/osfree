#
# A Makefile for udsk
# (c) osFree project,
#

PROJ     = udsk
DESC     = Low-level Disk Access API
srcfiles = $(p)secread$(e)

ADD_COPT = -s
ADD_LINKOPT  = op nod lib clib3r, os2386 segment   type DATA shared
DLL     = 1
DLLOPT  = initinstance terminstance

UNI2H = 1

EXPORTS = &
     DskSecRead             .1, &
     DskSecWrite            .2, &
     DskBootSecRead         .3, &
     DskBootSecWrite        .4, &
     DskMbrRead             .6, &
     DskMbrWrite            .7

DEST    = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2_cmd.mk
