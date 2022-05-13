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
OPTIONS = heapsize=32K libfile libentry.obj
DEST = os2$(SEP)mdos$(SEP)winos2$(SEP)system

!include $(%ROOT)/mk/appsw16.mk
