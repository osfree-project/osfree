#
# A Makefile for ansicall.dll
# (c) osFree project
#

PROJ     = audioif
TRGT     = $(PROJ).dll
DESC     = AudioIf
srcfiles = $(p)..$(SEP)shared$(SEP)stub$(e)
ADD_COPT = -i=..\include
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib segment   type DATA shared
DLL     = 1
DLLOPT  = initglobal
IMPORTS = VSDEntry              MMPM.152, &
	  AUDIOIFDriverEntry    MMPM.151, &
	  VSDLOGEntry		MMPM.227, &
	  VSDLOGSHDEntryPoint   MMPM.229
EXPORTS = VSDEntry.1,           &
	  AUDIOIFDriverEntry.2, &
	  VSDLOGEntry.3,        &
	  VSDLOGSHDEntryPoint.4

DEST    = mmos2$(SEP)dll

!include $(%ROOT)tools/mk/appsos2_cmd.mk
