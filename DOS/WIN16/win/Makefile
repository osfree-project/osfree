###########################################################################
#
#	@(#)Makefile	2.145 
#
#	Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.
#
#	Makefile for win directory.
#
###########################################################################

include $(TWINDIR)/Makefile.config
include $(TWINDIR)/makeinclude/Makefile.version

# Installation command, used to control owner and privleges.
INSTALL       = install -m 0644
INSTALL_EXEC  = install -m 0755

WINRT	= -ltwin $(TWIN_LIBS)
WINRT32	= -ltwin32 $(TWIN_LIBS)

############################################################################

TWIN_AUXINC  = -I.

############################################################################
#
#	the library objects
#

GDIOBJ	=	GdiObjects.o GdiBitmaps.o GdiClipping.o \
		GdiDC.o GdiBoundsRect.o GdiDCAttr.o GdiGraphics.o \
		GdiImages.o GdiBrute.o GdiMath.o \
		GdiText.o GdiRegions.o GdiPrinter.o FancyRgn.o \
		WSprintf.o DrawText.o Dither.o Caret.o \
		Palette.o MapMode.o ConvertArray.o \
		DisplayDC.o DisplayGraphics.o DisplayObjects.o \
		DisplayText.o DisplayClipping.o DeviceData.o LSDevices.o \
		Metafile.o Metaplay.o MetaGraphics.o MetaObjects.o \
		WImageIO.o WBmp.o

DRVOBJ  =	Driver.o

PRINTOBJ= 	PrinterGraphics.o PrintManager.o GdiTTEngine.o \
		PrinterDC.o

KERNOBJ = 	Kernel.o ObjEngine.o FileSys.o Net.o System.o \
		WinHelp.o SysMisc.o KrnAtoms.o KrnFileIO.o Events.o KrnTask.o \
		File32.o Threads.o Critical.o Interlock.o FormatMessage.o \
		CreateProcess.o Beep.o WinSockUtil.o Keyboard.o

USEROBJ	=	Resources.o  SysMetrics.o Dialog.o Profile.o Strings.o \
		MenuProc.o MenuAPI.o UsrCursor.o DefWnd.o Messages.o \
		Rect.o WSysMenu.o WinConfig.o Time.o Hotkey.o

WINOBJ  = 	WindowPos.o WindowBytes.o WindowCreate.o WindowProps.o \
		WindowRedraw.o WindowDC.o MDI.o Frame.o Hook.o Classes.o

IPCOBJ  = 	OleCli.o OleSvr.o O2CompObj.o O2GUID.o Clipboard.o 

CTLOBJ	= 	Listbox.o ComboBox.o \
		Button.o ButtonDraw.o Static.o Scroll.o ScrollPaint.o

CTLOBJ2	=	Edit.o EditKey.o EditDraw.o \
		EditMem.o EditMisc.o

RSCOBJ  = 	RscTemplate.o ConvertRes.o RscData.res.o 

LIBOBJ  = 	LoadLibrary.o ModTable.o WinInit.o IsBinary.o

DLLOBJ  = 	ToolHelp.o Comm.o Ver.o MessageBox.o 

MMSSYS  = 	MCI.o MultiMedia.o

DBGOBJ  = 	utils.o MsgCode.o

BINOBJ = 	LocalMem.o ConvertMsg.o Callback.o LoadDLL.o \
		GenericIF.o SpecialIF.o OleIF.o HookIF.o MemoryIF.o \
		VerIF.o CommIF.o PrintIF.o \
		PrinterBin.o BitmapBin.o OEMRoutines.o OEMTab.o \
		make_thunk.o invoke_binary.o DPMI.o ValidateModule.o \
		IOEmu.o KrnCatch.o


BINOBJ2 =	EditMemBin.o

# TWINOBJ defines those modules which cannot be part of the shared
# object on some platforms, due to architecture limitations.  This
# includes the Mac and the IBM/AIX platforms.  Do NOT make TWINOBJ
# part of ALLOBJ at any time.

TWINOBJ =	TwinDeflib.o TwinXWin.o

CPUOBJ_XXX	= $(CPUTYPE:INT16=intp.o DPMInotify.o) \
		  $(CPUTYPE:INT=intp32.o DPMInotify.o) \
		  $(CPUTYPE:CPU=Exec.o ExecUtl.o Exec$(EXECSUFFIX).o debugger.o readline.o)
CPUOBJ_XX	= $(CPUOBJ_XXX:INT16=)
CPUOBJ_X	= $(CPUOBJ_XX:INT=)
CPUOBJ		= $(CPUOBJ_X:CPU=)

MEMOBJ = 	$(TWINLIB)/Memory.o
XDOSOBJ = 	$(TWINLIB)/xdos.o
DRVX11OBJ =	$(TWINLIB)/x11.o
MMSYSTEMOBJ=	$(TWINLIB)/multimedia.o


APIOBJ	=	$(GDIOBJ) $(PRINTOBJ) $(KERNOBJ) \
		$(USEROBJ) $(SECOBJ) $(WINOBJ) $(IPCOBJ) \
		$(CTLOBJ) $(RSCOBJ) $(DLLOBJ) $(DBGOBJ) $(LIBOBJ)

APIOBJ2	=	$(CTLOBJ2)

#############################################################################
#
#	the library sources
#

INTSRC = 	Intp.c DPMInotify.c

EXECSRC	=	Exec.c  ExecUtl.S

ALLSRC	=	$(APIOBJ:.o=.c) \
		$(APIOBJ2:.o=.c) \
		$(BINOBJ:.o=.c) \
		$(BINOBJ2:.o=.c) \
		$(DRVOBJ:.o=.c) \
		$(INTSRC) \
	 	$(EXECSRC)

#	The ALLOBJ macro is, surprisingly, all of the object modules.
#	Well, almost.  We cannot include TWINOBJ in this macro, as it
#	must be handled specially on some platforms.

ALLOBJ  =       version.o $(APIOBJ) $(APIOBJ2) $(DRVOBJ) $(BINOBJ) $(BINOBJ2) \
		$(CPUOBJ)    \
		$(DRVX11OBJ) \
		$(XDOSOBJ)   \
		$(MMSYSTEMOBJ) \
		$(MEMOBJ)

ALLOBJ_SUNOS = version.o $(APIOBJ) $(APIOBJ2) $(DRVOBJ) $(BINOBJ) $(BINOBJ2)  \
		DPMInotify.o intp32/*.o \
		x11/*.o 		\
		xdos/*.o                \
		$(MMSYSTEMDIR)/*.o 	\
		memory/*.o

#############################################################################
#
#	objects needed for xwin
#

XWINOBJ =	WinMain.o TwinRT.res.o
TWIN =	twin.o TwinRT.res.o


#############################################################################
#
#	dependency rules for C++ files
#
.SUFFIXES:	.C
.C.o:
	$(C++) $(CFLAGS) -c $<

#############################################################################
#
#	header files 
#

APIHDR	=	ObjEngine.h Button.h Classes.h Edit.h Log.h \
		Kernel.h Listbox.h Menus.h Messages.h Meta.h \
		Objects.h Resources.h Scroll.h Static.h SysMetrics.h \
		GdiObjects.h kerndef.h DragDrop.h \
		Endian.h Log.h WinDefs.h WinData.h \
		KrnAtoms.h Dialog.h utils.h PrinterBin.h \
		GdiDC.h GdiRegions.h \
		Hook.h compat.h version.h

LIBHDR  = 	cderr.h colordlg.h commdlg.h custcntl.h ddeml.h dlgs.h \
		ole.h print.h shellapi.h windows.h windowsx.h ver.h dde.h \
		mmsystem.h toolhelp.h

BINHDR  = 	LoadEXE.h BinTypes.h Module.h \
		Intp.h LoadEXE.h Segment.h OEM.h DPMI.h

DRVHDR  =	Driver.h

HEADERS =	$(APIHDR) $(BINHDR) $(LIBHDR) $(DRVHDR)

MS_HEADERS= 	$(LIBHDR) penwin.h penwoem.h

# The SDK Include files to be moved into twin/include

SDK_HEADERS = 	$(MS_HEADERS) Resources.h compat.h dos.h \
		kerndef.h utils.h twindll.h

#############################################################################

all:	libtwin$(TWIN_APISIZE).so $(XWINOBJ)

build:	version $(LIBTWIN)

cpu:	$(CPUOBJ)

#############################################################################

xwin:	libtwin.so $(XWINOBJ)
	${CC} ${LDFLAGS} -o xwin $(XWINOBJ) $(WINRT)

xwin32: libtwin32.so $(XWINOBJ)
	${CC} ${LDFLAGS} -o xwin32 $(XWINOBJ) $(WINRT32)

libtwin.so: $(ALLOBJ) $(TWINOBJ)
	$(MAKE) $(PASSFLAGS) version
	$(MAKE) $(PASSFLAGS) version.o
	${CCSHARE} $(LDSHARED) -o libtwin.so.$(VERSION).$(PATCHLEVEL) $(ALLOBJ) $(TWINOBJ) $(SHARELIB)
	rm -f libtwin.so
	ln -s libtwin.so.$(VERSION).$(PATCHLEVEL) libtwin.so

libtwin32.so: $(ALLOBJ) $(TWINOBJ)
	$(MAKE) $(PASSFLAGS) version
	$(MAKE) $(PASSFLAGS) version.o
	${CCSHARE} $(LDSHARED) -o libtwin32.so.$(VERSION).$(PATCHLEVEL) $(ALLOBJ) $(TWINOBJ) $(SHARELIB)
	rm -f libtwin32.so
	ln -s libtwin32.so.$(VERSION).$(PATCHLEVEL) libtwin32.so


# Special link targets for HP/UX.

libtwin.sl: $(ALLOBJ) $(TWINOBJ)
	$(MAKE) $(PASSFLAGS) version
	$(MAKE) $(PASSFLAGS) version.o
	${CCSHARE} $(LDSHARED) -o libtwin.sl.$(VERSION).$(PATCHLEVEL) $(ALLOBJ) $(TWINOBJ) $(SHARELIB)
	rm -f libtwin.sl
	ln -s libtwin.sl.$(VERSION).$(PATCHLEVEL) libtwin.sl

libtwin32.sl: $(ALLOBJ) $(TWINOBJ)
	$(MAKE) $(PASSFLAGS) version
	$(MAKE) $(PASSFLAGS) version.o
	${CCSHARE} $(LDSHARED) -o libtwin32.sl.$(VERSION).$(PATCHLEVEL) \
		$(ALLOBJ) $(TWINOBJ) $(SHARELIB)
	rm -f libtwin32.sl
	ln -s libtwin32.sl.$(VERSION).$(PATCHLEVEL) libtwin32.sl


#  IBM/AIX method is a bit unusual.  Most object modules get built
#  into a single shared object.  The modules needed for main() and
#  its support (TWINOBJ) are built as normal objects.  These are
#  then all put into the library, which effectively becomes a 
#  library which is -mostly- shared.

libtwin_ibm.a: $(ALLOBJ) $(TWINOBJ) libtwin.exports
	rm -f libtwin.a libtwin_ibm.a libtwin.o
	$(MAKE) $(PASSFLAGS) version
	$(MAKE) $(PASSFLAGS) version.o
	$(CCSHARE) $(LDSHARED) -o libtwin.o $(ALLOBJ) $(SHARELIB)
	$(AR) r libtwin.a libtwin.o $(TWINOBJ)
	ln -s libtwin.a libtwin_ibm.a

libtwin32_ibm.a: $(ALLOBJ) $(TWINOBJ) libtwin32.exports
	rm -f libtwin32.a libtwin32_ibm.a libtwin32.o
	$(MAKE) $(PASSFLAGS) version
	$(MAKE) $(PASSFLAGS) version.o
	$(CCSHARE) $(LDSHARED32) -o libtwin32.o $(ALLOBJ) $(SHARELIB)
	$(AR) r libtwin32.a libtwin32.o $(TWINOBJ)
	ln -s libtwin32.a libtwin32_ibm.a


#  Sunos 4.1.3 method is non-standard, due to "ld -r" on PIC code not
#  not working properly.  (You get errors at link time on the output file.)
#  So we build everything normally, but at link time, bring in the .o files
#  from the subdirectory, not from the  "ld -r" output files.

libtwin_sunos.so: $(ALLOBJ) $(TWINOBJ)
	$(MAKE) $(PASSFLAGS) version
	$(MAKE) $(PASSFLAGS) version.o
	${CCSHARE} $(LDSHARED) -o libtwin.so.$(VERSION).$(PATCHLEVEL) \
	$(ALLOBJ_SUNOS) $(TWINOBJ) $(SHARELIB)
	ln -s libtwin.so.$(VERSION).$(PATCHLEVEL) libtwin.so
	ln -s libtwin.so.$(VERSION).$(PATCHLEVEL) libtwin_sunos.so

libtwin32_sunos.so: $(ALLOBJ) $(TWINOBJ)
	$(MAKE) $(PASSFLAGS) version
	$(MAKE) $(PASSFLAGS) version.o
	${CCSHARE} $(LDSHARED) -o libtwin32.so.$(VERSION).$(PATCHLEVEL) \
	$(ALLOBJ_SUNOS) $(TWINOBJ) $(SHARELIB)
	ln -s libtwin32.so.$(VERSION).$(PATCHLEVEL) libtwin32.so
	ln -s libtwin32.so.$(VERSION).$(PATCHLEVEL) libtwin32_sunos.so

##########################################################################
#
# Special Objects
#

ExecUtl.o: ExecUtl.S
	rm -f $*.s
	$(CC) $(TWIN_SYSINC) $(GFLAG) $(DEBUG) $(AUX) $(PLATFORM) -E $< > $*.s
	$(CC) $(TWIN_SYSINC) $(GFLAG) $(DEBUG) $(AUX) $(PLATFORM) -c $*.s

TwinRT.res.c: TwinRT.rc TwinRT.def
	$(RC) -I$(TWINDIR)/win -I$(TWINDIR)/include TwinRT.rc 

RscData.res.c: RscData.rc RscData.def
	$(RC) -I$(TWINDIR) -I$(TWINDIR)/win -I$(TWINDIR)/include RscData.rc 

xwinInit.c:
	linkdll -o xwinInit.c libcommdlg libshell

##########################################################################
#
# Make utilities
#

clean:
	rm -f *.a lib*.so* lib*.sl* 
	rm -f *.o 
	rm -f xwin* win286* win386* win486* core
	rm -f version.h version_time ExecUtl.s
	rm -f TwinRT.res.c RscData.res.c

clobber: clean

ctags:
	ctags *.c *.h >/dev/null 2>&1

depend:
	@-makedepend -f Makedepend.$(TARGET) $(DEPEND) -I. $(TWIN_SYSINC) $(TWIN_WININC) $(ALLSRC) 2>/dev/null

version: version_time
	@-/bin/sh ./makever.sh
	@-echo \#define WIN_RELEASE \"$(VERSION).$(PATCHLEVEL).$(ALPHA)\" > version.h
	@-echo \#define WIN_VERSION \"\#`cat .version` `date`\" >> version.h
	@-echo \#define WIN_COMPILE_BY \"`whoami`\"     >> version.h
	@-echo \#define WIN_COMPILE_HOST \"`hostname`\" >> version.h
	@-echo \#define WIN_DATE_SECS `./version_time` >> version.h
	@-echo \#define TWINOPT \"$(TWINOPT)\" >> version.h

version.h:
	$(MAKE) version

version.o: version.c version.h

version_time: version_time.c
	cc -o version_time version_time.c

LIBEXT_XX = $(LIBTWIN:libtwin.a=.a)
LIBEXT_X  = $(LIBEXT_XX:libtwin.so=.so)
LIBEXT    = $(LIBEXT_X:libtwin.sl=.sl)

install: xwin headers$(APPEND)
	strip xwin ; $(INSTALL_EXEC) xwin $(INSTALLDIR)/bin/xwin$(APPEND)
	$(INSTALL) $(LIBTWIN)  $(INSTALLDIR)/lib/libtwin$(APPEND)$(LIBEXT)

headers:
	$(INSTALL) $(SDK_HEADERS) $(INSTALLDIR)/include

headers_dbg:	# NOP to reduce copying
headers_prof:	# NOP to reduce copying

##########################################################################
#
# Source file dependencies are located in Makedepend.$(TARGET)
#
include Makedepend.$(TARGET)

##########################################################################
#
#
# DO NOT DELETE THIS LINE -- make depend depends on it.

