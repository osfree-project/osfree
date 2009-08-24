# Microsoft Visual C++ generated build script - Do not modify

PROJ = TVLIB
DEBUG = 1
PROGTYPE = 1
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = -d_DEBUG
R_RCDEFINES = -dNDEBUG
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = C:\WILLOWS\TWINVIEW\TVLIB\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = FILES.C     
FIRSTCPP =             
RC = rc
CFLAGS_D_WDLL = /nologo /G2 /W3 /Zi /ALw /Od /D "_DEBUG" /D "STRICT" /FR /GD /Fd"TVLIB.PDB"
CFLAGS_R_WDLL = /nologo /W3 /ALw /O1 /D "NDEBUG" /D "STRICT" /FR /GD 
LFLAGS_D_WDLL = /NOLOGO /NOD /NOE /PACKC:61440 /ALIGN:16 /ONERROR:NOEXE /CO /MAP:FULL
LFLAGS_R_WDLL = /NOLOGO /NOD /NOE /PACKC:61440 /ALIGN:16 /ONERROR:NOEXE /MAP:FULL
LIBS_D_WDLL = oldnames libw ldllcew commdlg.lib 
LIBS_R_WDLL = oldnames libw ldllcew commdlg.lib 
RCFLAGS = /nologo
RESFLAGS = /nologo
RUNFLAGS = 
DEFFILE = TVLIB.DEF
OBJS_EXT = 
LIBS_EXT = 
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_WDLL)
LFLAGS = $(LFLAGS_D_WDLL)
LIBS = $(LIBS_D_WDLL)
MAPFILE = nul
RCDEFINES = $(D_RCDEFINES)
!else
CFLAGS = $(CFLAGS_R_WDLL)
LFLAGS = $(LFLAGS_R_WDLL)
LIBS = $(LIBS_R_WDLL)
MAPFILE = nul
RCDEFINES = $(R_RCDEFINES)
!endif
!if [if exist MSVC.BND del MSVC.BND]
!endif
SBRS = FILES.SBR \
		LIBINFO.SBR \
		LIBLIST.SBR \
		MEMORY.SBR \
		MENULIST.SBR \
		OPEN.SBR \
		TVINI.SBR \
		TVLIB.SBR \
		WNDINFO.SBR \
		MESSAGE.SBR \
		EXTRBYTE.SBR \
		TVINFO.SBR


FILES_DEP = c:\willows\twinview\tvlib\tvlibprv.h \
	c:\willows\twinview\tvlib\tvlib.h


LIBINFO_DEP = c:\willows\twinview\tvlib\tvlibprv.h \
	c:\willows\twinview\tvlib\tvlib.h


LIBLIST_DEP = c:\willows\twinview\tvlib\tvlibprv.h \
	c:\willows\twinview\tvlib\tvlib.h


MEMORY_DEP = c:\willows\twinview\tvlib\tvlibprv.h \
	c:\willows\twinview\tvlib\tvlib.h


MENULIST_DEP = c:\willows\twinview\tvlib\tvlibprv.h \
	c:\willows\twinview\tvlib\tvlib.h


OPEN_DEP = c:\willows\twinview\tvlib\tvlibprv.h \
	c:\willows\twinview\tvlib\tvlib.h


TVINI_DEP = c:\willows\twinview\tvlib\tvlibprv.h \
	c:\willows\twinview\tvlib\tvlib.h


TVLIB_DEP = c:\willows\twinview\tvlib\tvlibprv.h \
	c:\willows\twinview\tvlib\tvlib.h


WNDINFO_DEP = c:\willows\twinview\tvlib\tvlibprv.h \
	c:\willows\twinview\tvlib\tvlib.h


MESSAGE_DEP = c:\willows\twinview\tvlib\tvlibprv.h \
	c:\willows\twinview\tvlib\tvlib.h


EXTRBYTE_DEP = c:\willows\twinview\tvlib\tvlibprv.h \
	c:\willows\twinview\tvlib\tvlib.h


TVINFO_DEP = c:\willows\twinview\tvlib\tvlibprv.h \
	c:\willows\twinview\tvlib\tvlib.h


all:	$(PROJ).DLL $(PROJ).BSC

FILES.OBJ:	FILES.C $(FILES_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c FILES.C

LIBINFO.OBJ:	LIBINFO.C $(LIBINFO_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c LIBINFO.C

LIBLIST.OBJ:	LIBLIST.C $(LIBLIST_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c LIBLIST.C

MEMORY.OBJ:	MEMORY.C $(MEMORY_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c MEMORY.C

MENULIST.OBJ:	MENULIST.C $(MENULIST_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c MENULIST.C

OPEN.OBJ:	OPEN.C $(OPEN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c OPEN.C

TVINI.OBJ:	TVINI.C $(TVINI_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c TVINI.C

TVLIB.OBJ:	TVLIB.C $(TVLIB_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c TVLIB.C

WNDINFO.OBJ:	WNDINFO.C $(WNDINFO_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WNDINFO.C

MESSAGE.OBJ:	MESSAGE.C $(MESSAGE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c MESSAGE.C

EXTRBYTE.OBJ:	EXTRBYTE.C $(EXTRBYTE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c EXTRBYTE.C

TVINFO.OBJ:	TVINFO.C $(TVINFO_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c TVINFO.C


$(PROJ).DLL::	FILES.OBJ LIBINFO.OBJ LIBLIST.OBJ MEMORY.OBJ MENULIST.OBJ OPEN.OBJ \
	TVINI.OBJ TVLIB.OBJ WNDINFO.OBJ MESSAGE.OBJ EXTRBYTE.OBJ TVINFO.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
FILES.OBJ +
LIBINFO.OBJ +
LIBLIST.OBJ +
MEMORY.OBJ +
MENULIST.OBJ +
OPEN.OBJ +
TVINI.OBJ +
TVLIB.OBJ +
WNDINFO.OBJ +
MESSAGE.OBJ +
EXTRBYTE.OBJ +
TVINFO.OBJ +
$(OBJS_EXT)
$(PROJ).DLL
$(MAPFILE)
c:\msvc\lib\+
c:\msvc\mfc\lib\+
C:\willows\twinview\tvlib\+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) $@
	implib /nowep $(PROJ).LIB $(PROJ).DLL


run: $(PROJ).DLL
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
