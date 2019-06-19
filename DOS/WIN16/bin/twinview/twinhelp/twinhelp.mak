# Microsoft Visual C++ generated build script - Do not modify

PROJ = TWINHELP
DEBUG = 1
PROGTYPE = 1
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = /d_DEBUG 
R_RCDEFINES = /dNDEBUG 
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = C:\WILLOWS\TWINVIEW\TWINHELP\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = TWINHELP.C  
FIRSTCPP =             
RC = rc
CFLAGS_D_WDLL = /nologo /G2 /Zp1 /W3 /Zi /ALw /Od /D "_DEBUG" /D "STRICT" /FR /GD /Fd"TWINHELP.PDB"
CFLAGS_R_WDLL = /nologo /G2 /Zp1 /W3 /ALw /O2 /D "NDEBUG" /D "STRICT" /FR /GD 
LFLAGS_D_WDLL = /NOLOGO /NOD /NOE /PACKC:61440 /ALIGN:16 /ONERROR:NOEXE /CO /MAP:FULL
LFLAGS_R_WDLL = /NOLOGO /NOD /NOE /PACKC:61440 /ALIGN:16 /ONERROR:NOEXE /MAP:FULL
LIBS_D_WDLL = oldnames libw ldllcew commdlg.lib 
LIBS_R_WDLL = oldnames libw ldllcew commdlg.lib 
RCFLAGS = /nologo 
RESFLAGS = /nologo 
RUNFLAGS = 
DEFFILE = TWINHELP.DEF
OBJS_EXT = 
LIBS_EXT = ..\TVLIB\TVLIB.LIB 
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
SBRS = TWINHELP.SBR \
		GLOBALS.SBR \
		MSGBOX.SBR \
		WNDLIST.SBR \
		LIBMAIN.SBR \
		BTNBAR.SBR \
		WINMEM.SBR \
		WNDDATA.SBR \
		DIALOGS.SBR \
		HLPFILE.SBR \
		FONTBUFF.SBR \
		ELEMENTS.SBR \
		FILEBUFF.SBR \
		DISPLAY.SBR \
		INIFILE.SBR \
		HISTORY.SBR \
		HOTSPOTS.SBR \
		BACK.SBR \
		PORTABLE.SBR \
		SEARCH.SBR \
		WNDPOS.SBR \
		SECWND.SBR \
		MACROENG.SBR \
		HLPMACRO.SBR \
		ACCEL.SBR


TVLIB_DEP = 

TWINHELP_DEP = c:\willows\twinview\twinhelp\twinhelp.h \
	c:\willows\twinview\twinhelp\msgbox.h \
	c:\willows\twinview\twinhelp\winmem.h \
	c:\willows\twinview\twinhelp\globals.h \
	c:\willows\twinview\twinhelp\wndlist.h \
	c:\willows\twinview\twinhelp\helpapi.h \
	c:\willows\twinview\twinhelp\btnbar.h \
	c:\willows\twinview\twinhelp\wnddata.h \
	c:\willows\twinview\twinhelp\dialogs.h \
	c:\willows\twinview\twinhelp\fontbuff.h \
	c:\willows\twinview\twinhelp\filebuff.h \
	c:\willows\twinview\twinhelp\hlpfile.h \
	c:\willows\twinview\twinhelp\display.h \
	c:\willows\twinview\twinhelp\hotspots.h \
	c:\willows\twinview\twinhelp\elements.h \
	c:\willows\twinview\twinhelp\history.h \
	c:\willows\twinview\twinhelp\back.h \
	c:\willows\twinview\twinhelp\inifile.h \
	c:\willows\twinview\twinhelp\wndpos.h \
	c:\willows\twinview\twinhelp\secwnd.h \
	c:\willows\twinview\twinhelp\hlpmacro.h \
	c:\willows\twinview\twinhelp\accel.h


TWINHELP_RCDEP = c:\willows\twinview\twinhelp\hand.cur \
	c:\willows\twinview\twinhelp\twinhelp.ico


GLOBALS_DEP = c:\willows\twinview\twinhelp\globals.h \
	c:\willows\twinview\twinhelp\wndlist.h


MSGBOX_DEP = c:\willows\twinview\twinhelp\msgbox.h


WNDLIST_DEP = c:\willows\twinview\twinhelp\wndlist.h \
	c:\willows\twinview\twinhelp\msgbox.h \
	c:\willows\twinview\twinhelp\winmem.h


LIBMAIN_DEP = c:\willows\twinview\twinhelp\twinhelp.h \
	c:\willows\twinview\twinhelp\msgbox.h \
	c:\willows\twinview\twinhelp\globals.h \
	c:\willows\twinview\twinhelp\wndlist.h \
	c:\willows\twinview\tvlib\tvlib.h \
	c:\willows\twinview\twinhelp\helpapi.h \
	c:\willows\twinview\twinhelp\btnbar.h \
	c:\willows\twinview\twinhelp\inifile.h \
	c:\willows\twinview\twinhelp\wndpos.h


BTNBAR_DEP = c:\willows\twinview\twinhelp\btnbar.h \
	c:\willows\twinview\twinhelp\winmem.h \
	c:\willows\twinview\twinhelp\twinhelp.h \
	c:\willows\twinview\twinhelp\hlpmacro.h \
	c:\willows\twinview\twinhelp\msgbox.h


WINMEM_DEP = c:\willows\twinview\twinhelp\winmem.h \
	c:\willows\twinview\twinhelp\msgbox.h \
	c:\willows\twinview\twinhelp\globals.h \
	c:\willows\twinview\twinhelp\wndlist.h


WNDDATA_DEP = c:\willows\twinview\twinhelp\wnddata.h \
	c:\willows\twinview\twinhelp\fontbuff.h \
	c:\willows\twinview\twinhelp\winmem.h \
	c:\willows\twinview\twinhelp\macroeng.h \
	c:\willows\twinview\twinhelp\filebuff.h \
	c:\willows\twinview\twinhelp\hlpfile.h \
	c:\willows\twinview\twinhelp\accel.h


DIALOGS_DEP = c:\willows\twinview\twinhelp\globals.h \
	c:\willows\twinview\twinhelp\wndlist.h \
	c:\willows\twinview\twinhelp\dialogs.h


HLPFILE_DEP = c:\willows\twinview\twinhelp\hlpfile.h \
	c:\willows\twinview\twinhelp\winmem.h \
	c:\willows\twinview\twinhelp\msgbox.h \
	c:\willows\twinview\twinhelp\globals.h \
	c:\willows\twinview\twinhelp\wndlist.h \
	c:\willows\twinview\twinhelp\wnddata.h \
	c:\willows\twinview\twinhelp\elements.h \
	c:\willows\twinview\twinhelp\filebuff.h \
	c:\willows\twinview\twinhelp\wndpos.h \
	c:\willows\twinview\twinhelp\inifile.h


FONTBUFF_DEP = c:\willows\twinview\twinhelp\fontbuff.h \
	c:\willows\twinview\twinhelp\winmem.h \
	c:\willows\twinview\twinhelp\wnddata.h


ELEMENTS_DEP = c:\willows\twinview\twinhelp\msgbox.h \
	c:\willows\twinview\twinhelp\globals.h \
	c:\willows\twinview\twinhelp\wndlist.h \
	c:\willows\twinview\twinhelp\wnddata.h \
	c:\willows\twinview\twinhelp\elements.h \
	c:\willows\twinview\twinhelp\hlpfile.h \
	c:\willows\twinview\twinhelp\filebuff.h \
	c:\willows\twinview\twinhelp\winmem.h \
	c:\willows\twinview\twinhelp\portable.h


FILEBUFF_DEP = c:\willows\twinview\twinhelp\filebuff.h \
	c:\willows\twinview\twinhelp\hlpfile.h \
	c:\willows\twinview\twinhelp\msgbox.h \
	c:\willows\twinview\twinhelp\globals.h \
	c:\willows\twinview\twinhelp\wndlist.h \
	c:\willows\twinview\twinhelp\wnddata.h \
	c:\willows\twinview\twinhelp\winmem.h \
	c:\willows\twinview\twinhelp\elements.h


DISPLAY_DEP = c:\willows\twinview\twinhelp\winmem.h \
	c:\willows\twinview\twinhelp\wnddata.h \
	c:\willows\twinview\twinhelp\filebuff.h \
	c:\willows\twinview\twinhelp\hlpfile.h \
	c:\willows\twinview\twinhelp\fontbuff.h \
	c:\willows\twinview\twinhelp\elements.h \
	c:\willows\twinview\twinhelp\display.h \
	c:\willows\twinview\twinhelp\hotspots.h \
	c:\willows\twinview\twinhelp\twinhelp.h \
	c:\willows\twinview\twinhelp\inifile.h \
	c:\willows\twinview\twinhelp\globals.h \
	c:\willows\twinview\twinhelp\wndlist.h


INIFILE_DEP = c:\willows\twinview\twinhelp\inifile.h \
	c:\willows\twinview\twinhelp\twinhelp.h


HISTORY_DEP = c:\willows\twinview\twinhelp\history.h \
	c:\willows\twinview\twinhelp\winmem.h \
	c:\willows\twinview\twinhelp\globals.h \
	c:\willows\twinview\twinhelp\wndlist.h \
	c:\willows\twinview\twinhelp\msgbox.h \
	c:\willows\twinview\twinhelp\twinhelp.h \
	c:\willows\twinview\twinhelp\inifile.h \
	c:\willows\twinview\twinhelp\filebuff.h \
	c:\willows\twinview\twinhelp\hlpfile.h \
	c:\willows\twinview\twinhelp\wnddata.h


HOTSPOTS_DEP = c:\willows\twinview\twinhelp\hotspots.h \
	c:\willows\twinview\twinhelp\twinhelp.h \
	c:\willows\twinview\twinhelp\elements.h \
	c:\willows\twinview\twinhelp\hlpfile.h \
	c:\willows\twinview\twinhelp\filebuff.h \
	c:\willows\twinview\twinhelp\winmem.h


BACK_DEP = c:\willows\twinview\twinhelp\back.h \
	c:\willows\twinview\twinhelp\filebuff.h \
	c:\willows\twinview\twinhelp\winmem.h \
	c:\willows\twinview\twinhelp\inifile.h \
	c:\willows\twinview\twinhelp\wnddata.h \
	c:\willows\twinview\twinhelp\btnbar.h \
	c:\willows\twinview\twinhelp\twinhelp.h


PORTABLE_DEP = c:\willows\twinview\twinhelp\portable.h


SEARCH_DEP = c:\willows\twinview\twinhelp\hlpfile.h \
	c:\willows\twinview\twinhelp\display.h \
	c:\willows\twinview\twinhelp\fontbuff.h \
	c:\willows\twinview\twinhelp\hotspots.h \
	c:\willows\twinview\twinhelp\twinhelp.h \
	c:\willows\twinview\twinhelp\elements.h \
	c:\willows\twinview\twinhelp\filebuff.h \
	c:\willows\twinview\twinhelp\winmem.h \
	c:\willows\twinview\twinhelp\wnddata.h \
	c:\willows\twinview\twinhelp\globals.h \
	c:\willows\twinview\twinhelp\wndlist.h


WNDPOS_DEP = c:\willows\twinview\twinhelp\wndpos.h


SECWND_DEP = c:\willows\twinview\twinhelp\secwnd.h \
	c:\willows\twinview\twinhelp\filebuff.h \
	c:\willows\twinview\twinhelp\msgbox.h \
	c:\willows\twinview\twinhelp\winmem.h \
	c:\willows\twinview\twinhelp\hlpfile.h \
	c:\willows\twinview\twinhelp\wnddata.h \
	c:\willows\twinview\twinhelp\twinhelp.h \
	c:\willows\twinview\twinhelp\globals.h \
	c:\willows\twinview\twinhelp\wndlist.h


MACROENG_DEP = c:\willows\twinview\twinhelp\macroeng.h \
	c:\willows\twinview\twinhelp\winmem.h \
	c:\willows\twinview\twinhelp\msgbox.h \
	c:\willows\twinview\twinhelp\globals.h \
	c:\willows\twinview\twinhelp\wndlist.h


HLPMACRO_DEP = c:\willows\twinview\twinhelp\macroeng.h \
	c:\willows\twinview\twinhelp\hlpmacro.h \
	c:\willows\twinview\twinhelp\msgbox.h \
	c:\willows\twinview\twinhelp\globals.h \
	c:\willows\twinview\twinhelp\wndlist.h \
	c:\willows\twinview\twinhelp\wnddata.h \
	c:\willows\twinview\twinhelp\winmem.h \
	c:\willows\twinview\twinhelp\twinhelp.h \
	c:\willows\twinview\twinhelp\btnbar.h \
	c:\willows\twinview\twinhelp\hlpfile.h \
	c:\willows\twinview\twinhelp\filebuff.h \
	c:\willows\twinview\twinhelp\accel.h


ACCEL_DEP = c:\willows\twinview\twinhelp\accel.h \
	c:\willows\twinview\twinhelp\winmem.h \
	c:\willows\twinview\twinhelp\globals.h \
	c:\willows\twinview\twinhelp\wndlist.h \
	c:\willows\twinview\twinhelp\msgbox.h \
	c:\willows\twinview\twinhelp\wnddata.h \
	c:\willows\twinview\twinhelp\hlpmacro.h


all:	$(PROJ).DLL $(PROJ).BSC

TWINHELP.OBJ:	TWINHELP.C $(TWINHELP_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c TWINHELP.C

TWINHELP.RES:	TWINHELP.RC $(TWINHELP_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r TWINHELP.RC

GLOBALS.OBJ:	GLOBALS.C $(GLOBALS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c GLOBALS.C

MSGBOX.OBJ:	MSGBOX.C $(MSGBOX_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c MSGBOX.C

WNDLIST.OBJ:	WNDLIST.C $(WNDLIST_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WNDLIST.C

LIBMAIN.OBJ:	LIBMAIN.C $(LIBMAIN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c LIBMAIN.C

BTNBAR.OBJ:	BTNBAR.C $(BTNBAR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c BTNBAR.C

WINMEM.OBJ:	WINMEM.C $(WINMEM_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WINMEM.C

WNDDATA.OBJ:	WNDDATA.C $(WNDDATA_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WNDDATA.C

DIALOGS.OBJ:	DIALOGS.C $(DIALOGS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c DIALOGS.C

HLPFILE.OBJ:	HLPFILE.C $(HLPFILE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c HLPFILE.C

FONTBUFF.OBJ:	FONTBUFF.C $(FONTBUFF_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c FONTBUFF.C

ELEMENTS.OBJ:	ELEMENTS.C $(ELEMENTS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ELEMENTS.C

FILEBUFF.OBJ:	FILEBUFF.C $(FILEBUFF_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c FILEBUFF.C

DISPLAY.OBJ:	DISPLAY.C $(DISPLAY_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c DISPLAY.C

INIFILE.OBJ:	INIFILE.C $(INIFILE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c INIFILE.C

HISTORY.OBJ:	HISTORY.C $(HISTORY_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c HISTORY.C

HOTSPOTS.OBJ:	HOTSPOTS.C $(HOTSPOTS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c HOTSPOTS.C

BACK.OBJ:	BACK.C $(BACK_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c BACK.C

PORTABLE.OBJ:	PORTABLE.C $(PORTABLE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c PORTABLE.C

SEARCH.OBJ:	SEARCH.C $(SEARCH_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c SEARCH.C

WNDPOS.OBJ:	WNDPOS.C $(WNDPOS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WNDPOS.C

SECWND.OBJ:	SECWND.C $(SECWND_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c SECWND.C

MACROENG.OBJ:	MACROENG.C $(MACROENG_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c MACROENG.C

HLPMACRO.OBJ:	HLPMACRO.C $(HLPMACRO_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c HLPMACRO.C

ACCEL.OBJ:	ACCEL.C $(ACCEL_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ACCEL.C


$(PROJ).DLL::	TWINHELP.RES

$(PROJ).DLL::	TWINHELP.OBJ GLOBALS.OBJ MSGBOX.OBJ WNDLIST.OBJ LIBMAIN.OBJ BTNBAR.OBJ \
	WINMEM.OBJ WNDDATA.OBJ DIALOGS.OBJ HLPFILE.OBJ FONTBUFF.OBJ ELEMENTS.OBJ FILEBUFF.OBJ \
	DISPLAY.OBJ INIFILE.OBJ HISTORY.OBJ HOTSPOTS.OBJ BACK.OBJ PORTABLE.OBJ SEARCH.OBJ \
	WNDPOS.OBJ SECWND.OBJ MACROENG.OBJ HLPMACRO.OBJ ACCEL.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
TWINHELP.OBJ +
GLOBALS.OBJ +
MSGBOX.OBJ +
WNDLIST.OBJ +
LIBMAIN.OBJ +
BTNBAR.OBJ +
WINMEM.OBJ +
WNDDATA.OBJ +
DIALOGS.OBJ +
HLPFILE.OBJ +
FONTBUFF.OBJ +
ELEMENTS.OBJ +
FILEBUFF.OBJ +
DISPLAY.OBJ +
INIFILE.OBJ +
HISTORY.OBJ +
HOTSPOTS.OBJ +
BACK.OBJ +
PORTABLE.OBJ +
SEARCH.OBJ +
WNDPOS.OBJ +
SECWND.OBJ +
MACROENG.OBJ +
HLPMACRO.OBJ +
ACCEL.OBJ +
$(OBJS_EXT)
$(PROJ).DLL
$(MAPFILE)
c:\msvc\lib\+
c:\msvc\mfc\lib\+
C:\willows\twinview\tvlib\+
..\TVLIB\TVLIB.LIB+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) TWINHELP.RES $@
	@copy $(PROJ).CRF MSVC.BND
	implib /nowep $(PROJ).LIB $(PROJ).DLL

$(PROJ).DLL::	TWINHELP.RES
	if not exist MSVC.BND 	$(RC) $(RESFLAGS) TWINHELP.RES $@

run: $(PROJ).DLL
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
