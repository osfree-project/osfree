#
# A Makefile for lzexpand.dll
# (c) osFree project,
# author, date
#

PROJ     = lzexpand
DESC     = Windows file expansion library
srcfiles = $(p)lzexpand$(e)
ADD_COPT = -bt=windiws -bd
ADD_LINKOPT = lib clibs.lib,windows.lib,clibdls.lib
# linking dll file
DLL     = 1
EXPORTS = LZCOPY          .1, &
          LZOPENFILE      .2, &
          LZINIT          .3, &
          LZSEEK          .4, &
          LZREAD          .5, &
          LZCLOSE         .6, &
          LZSTART         .7, &
          COPYLZFILE      .8, &
          LZDONE          .9, &
          GETEXPANDEDNAME .10
OPTIONS = heapsize=32K libfile libentry

!include $(%ROOT)/mk/win16app.mk
