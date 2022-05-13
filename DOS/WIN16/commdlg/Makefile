#
# A Makefile for lzexpand.dll
# (c) osFree project,
# author, date
#

PROJ     = commdlg
TRGT     = $(PROJ).dll
DESC     = Windows common dialogs library
srcfiles = $(p)ChooseColor$(e) $(p)ChooseFont$(e) $(p)Commdlg$(e) $(p)FindReplDlg$(e) $(p)GetOpenFileName$(e) $(p)PrintDlg$(e) $(p)WinConfig$(e) #$(p)CommdlgIF$(e) $(p)CommdlgIT$(e)

# linking dll file
DLL     = 1
EXPORTS = GETOPENFILENAME.1, &
GETSAVEFILENAME.2, &
CHOOSECOLOR.5, &
FILEOPENDLGPROC.6, &
FILESAVEDLGPROC.7, &
COLORDLGPROC.8, &
LOADALTERBITMAP.9, &
FINDTEXT.11, &
REPLACETEXT.12, &
FINDTEXTDLGPROC.13, &
REPLACETEXTDLGPROC.14, &
CHOOSEFONT.15, &
FORMATCHARDLGPROC.16, &
FONTSTYLEENUMPROC.18, &
FONTFAMILYENUMPROC.19, &
PRINTDLG.20, &
PRINTDLGPROC.21, &
PRINTSETUPDLGPROC.22, &
EDITINTEGERONLY.23, &
WANTARROWS.25, &
COMMDLGEXTENDEDERROR.26, &
GETFILETITLE.27, &
DWLBSUBCLASS.29, &
DWUPARROWHACK.30, &
DWOKSUBCLASS.31

OPTIONS = heapsize=32K libfile libentry.obj
DEST = os2$(SEP)mdos$(SEP)winos2$(SEP)system

!include $(%ROOT)/mk/appsw16.mk
