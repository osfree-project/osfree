#
# A Makefile for midimcd.dll
# (c) osFree project
#

PROJ     = midimcd
TRGT     = $(PROJ).dll
DESC     = MIDI MCI DLL
srcfiles = $(p)..$(SEP)shared$(SEP)stub$(e)
ADD_COPT = -i=..\include
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib segment   type DATA shared
DLL     = 1
DLLOPT  = initglobal
IMPORTS = mciDriverEntry         MMPM.154, &
	  SoftSynthPageDlgProc   MMPM.226
EXPORTS = mciDriverEntry.1, &
	  SoftSynthPageDlgProc.2

DEST    = mmos2$(SEP)dll

!include $(%ROOT)tools/mk/appsos2_cmd.mk
