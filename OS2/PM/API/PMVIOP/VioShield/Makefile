.ERASE

.SUFFIXES :
.SUFFIXES : .exe .dll .res .obj .h .asm .c .dlg .rc

AS = wasm
AFLAGS = -zq

CC = wcc386
#CFLAGS = -zq -wx -we -bm
CFLAGS = -zq -wx -bm

LINK = wlink
LFLAGS = option quiet, map

!ifdef RELEASE
AFLAGS += -d0
CFLAGS += -d0 -oaxt
!else
AFLAGS += -d2
CFLAGS += -d2 -DDEBUG
LFLAGS += debug watcom all
!endif

RC = wrc
RCFLAGS = -q

ZIP = zip

DEL = del

.asm.obj :
	$(AS) $(AFLAGS) -fo=$@ $[@

.c.obj :
	$(CC) $(CFLAGS) -fo=$@ $[@

.rc.res :
	$(RC) $(RCFLAGS) -r $[@ $@

KSHELL_OBJS = kshell.obj ft2lib.obj

all : .SYMBOLIC viodmn.exe kshell.exe kshell.res viosub.dll test.exe

kshell.exe : $(KSHELL_OBJS) kshell.res
	$(LINK) $(LFLAGS) system os2v2_pm library libuls, libconv name $@ file { $(KSHELL_OBJS) }
	$(RC) $(RCFLAGS) kshell.res -fe=$@

viodmn.exe : viodmn.obj
	$(LINK) $(LFLAGS) system os2v2 name $@ file { $< }

test.exe : test.obj
	$(LINK) $(LFLAGS) system os2v2 name $@ file { $< }

kshell.res : kshell.rc kshell.h kshell.ico cpdlg.h cpdlg.dlg

VIOSUB_OBJS = viosub.obj vioroute.obj

viosub.dll : $(VIOSUB_OBJS) viosub.lnk
	$(LINK) $(LFLAGS) @viosub.lnk name $@ file { $(VIOSUB_OBJS) }

kshell.obj : kshell.c kshell.h cpdlg.h viodmn.h viosub.h

ft2lib.obj : ft2lib.c ft2lib.h

viodmn.obj : viodmn.c kshell.h cpdlg.h viodmn.h

viosub.obj : viosub.c dosqss.h viosub.h kshell.h cpdlg.h
	$(CC) $(CFLAGS) -s -fo=$@ $[@

vioroute.obj : vioroute.asm

test.obj : test.c

dist : .SYMBOLIC
	$(MAKE) clean
	$(MAKE) bin RELEASE=1 VER=$(VER)
	$(MAKE) src RELEASE=1
	$(ZIP) kshell$(VER) kshellsrc.zip
	-$(DEL) kshellsrc.zip

bin : .SYMBOLIC kshell.exe viodmn.exe viosub.dll test.exe readme.txt readme.eng
	-$(DEL) kshell$(VER).zip
	$(ZIP) kshell$(VER) $<

src : .SYMBOLIC kshell.c kshell.h kshell.rc kshell.ico ft2lib.c ft2lib.h &
	  viodmn.c viodmn.h viosub.c viosub.h vioroute.asm viosub.lnk &
	  dosqss.h test.c cpdlg.dlg cpdlg.h &
	  makefile
	-$(DEL) kshellsrc.zip
	$(ZIP) kshellsrc $<

clean : .SYMBOLIC
	-$(DEL) *.map
	-$(DEL) *.obj
	-$(DEL) *.dll
	-$(DEL) *.exe
	-$(DEL) *.res
	-$(DEL) *.zip
	-$(DEL) *~?

