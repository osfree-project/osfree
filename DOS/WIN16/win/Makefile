#
# A Makefile for ATTRIB
# (c) osFree project,
# author, date
#

PROJ      = win
DESC      = Win16 main file
srcfiles  = &
$(p)WinMain$(e) &
$(p)TwinRT.res$(e) &
$(p)IsBinary$(e) &
$(p)TwinDeflib$(e) &
$(p)Kernel$(e) &
$(p)WinInit$(e) &
$(p)WinConfig$(e) &
$(p)Strings$(e) &
$(p)LoadLibrary$(e) &
$(p)LoadDLL$(e) &
$(p)ModTable$(e) &
$(p)MessageBox$(e) &
$(p)KrnAtoms$(e) &
$(p)KrnTask$(e) &
$(p)KrnCatch$(e) &
$(p)KrnFileIO$(e) &
$(p)ObjEngine$(e) &
$(p)WindowPos$(e) &
$(p)WindowRedraw$(e) &
$(p)WSprintf$(e) &
$(p)SysMetrics$(e) &
$(p)FancyRgn$(e) &
$(p)Messages$(e) &
$(p)ConvertMsg$(e) &
$(p)Events$(e) &
$(p)Resources$(e) &
$(p)ConvertRes$(e) &
$(p)LSDevices$(e) &
$(p)FileSys$(e) &
$(p)Edit$(e) &
$(p)EditKey$(e) &
$(p)EditMem$(e) &
$(p)EditMisc$(e) &
$(p)File32$(e) &
$(p)utils$(e) &
$(p)GenericIF$(e) &
$(p)SpecialIF$(e) &
$(p)Driver$(e) &
$(p)Callback$(e) &
$(p)GdiDC$(e) &
$(p)GdiDCAttr$(e) &
$(p)GdiGraphics$(e) &
$(p)GdiObjects$(e) &
$(p)GdiMath$(e) &
$(p)GdiRegions$(e) &
$(p)GdiPrinter$(e) &
$(p)GdiBrute$(e) &
$(p)Profile$(e) &
$(p)ListBox$(e) &
$(p)Palette$(e) &
$(p)PrinterBin$(e) &
$(p)Metafile$(e) &
$(p)Metaplay$(e) &
$(p)Classes$(e) &
$(p)LocalMem$(e) &
$(p)MemoryIF$(e) &
$(p)WSysMenu$(e) &
$(p)System$(e) &
$(p)BitmapBin$(e) &
$(p)MapMode$(e) &
$(p)DPMI$(e) &
$(p)DPMInotify$(e) &
$(p)OEMRoutines$(e) &
$(p)invoke_binary$(e) &
$(p)EditMemBin$(e) &
$(p)MenuAPI$(e) &
$(p)Beep$(e) &
$(p)Button$(e) &
$(p)ButtonDraw$(e) &
$(p)Caret$(e) &
$(p)Clipboard$(e) &
$(p)ComboBox$(e) &
$(p)CommIF$(e) &
$(p)ConvertArray$(e) &
$(p)CreateProcess$(e) &
$(p)Critical$(e) &
$(p)DefWnd$(e) &
$(p)DeviceData$(e) &
$(p)DisplayClipping$(e) &
$(p)DisplayDC$(e) &
$(p)DisplayGraphics$(e) &
$(p)DisplayObjects$(e) &
$(p)DisplayText$(e) &
$(p)Dither$(e) &
$(p)DrawText$(e) &
$(p)EditDraw$(e) &
$(p)FormatMessage$(e) &
$(p)Frame$(e) &
$(p)GdiBitmaps$(e) &
$(p)GdiBoundsRect$(e) &
$(p)GdiClipping$(e) &
$(p)GdiImages$(e) &
$(p)GdiText$(e) &
$(p)GdiTTEngine$(e) &
$(p)Hook$(e) &
$(p)HookIF$(e) &
$(p)Hotkey$(e) &
$(p)Interlock$(e) &
$(p)IOEmu$(e) &
$(p)Keyboard$(e) &
$(p)MDI$(e) &
$(p)MenuProc$(e) &
$(p)MetaGraphics$(e) &
$(p)MetaObjects$(e) &
$(p)MsgCode$(e) &
$(p)Net$(e) &
$(p)O2CompObj$(e) &
$(p)O2GUID$(e) &
$(p)OEMTab$(e) &
$(p)OleCli$(e) &
$(p)OleIF$(e) &
$(p)OleSvr$(e) &
$(p)PrinterDC$(e) &
$(p)PrinterGraphics$(e) &
$(p)PrintIF$(e) &
$(p)Rect$(e) &
$(p)RscTable$(e) &
$(p)RscTemplate$(e) &
$(p)Scroll$(e) &
$(p)ScrollPaint$(e) &
$(p)Static$(e) &
$(p)SysMisc$(e) &
$(p)Threads$(e) &
$(p)Time$(e) &
$(p)ToolHelp$(e) &
$(p)UsrCursor$(e) &
$(p)ValidateModule$(e) &
$(p)Ver$(e) &
$(p)VerIF$(e) &
$(p)version$(e) &
$(p)WBmp$(e) &
$(p)WImageIO$(e) &
$(p)WindowBytes$(e) &
$(p)WindowCreate$(e) &
$(p)WindowDC$(e) &
$(p)WindowProps$(e) &
$(p)Dialog$(e) &
$(p)PrintManager$(e) &
$(p)WinHelp$(e) &
$(p)Comm$(e)

#$(p)make_thunk$(e) &


ADD_COPT = -i=$(MYDIR)..$(SEP)include -i=$(%WATCOM)$(SEP)h -i=$(%WATCOM)$(SEP)h$(SEP)dos

# defines additional options for C compiler

!include $(%ROOT)/mk/appsdos.mk

.c: $(PATH)

$(PATH)TwinRT.res.obj: $(PATH)TwinRT.res.c

$(PATH)TwinRT.res.c: $(MYDIR)TwinRT.rc
   winrc -I$(MYDIR)..$(SEP)include -I. $< -o $^@
