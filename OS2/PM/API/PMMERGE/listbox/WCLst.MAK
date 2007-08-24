
# MAKE file for WCLst.MAK     Version 1.1

# Revised:  1994-04-21

# Copyright ¸ 1987-1994  Prominare Inc.

# MAKE file created by Prominare Builder  Version 2.1b

# Macro definitions

CC=Wcl386
RC=rc
C_SW=-c -ms -3s -bd -s -od -d2 -zp4 -w3
RC_SW=-r


ListBox.Dll: EventMgr.Obj heapmgr.Obj listbox.Obj\
             Listbox.Res listmgr.Obj SndMgr.Obj\
             TraffMgr.Obj EntryFld.Obj
 WLink DEBUG ALL @WCLst.LNK
 rc listbox.res listbox.dll

EventMgr.Obj: EventMgr.C
 $(CC) $(C_SW)-fo=$*.Obj EventMgr.C

heapmgr.Obj: heapmgr.c
 $(CC) $(C_SW)-fo=$*.Obj heapmgr.c

listbox.Obj: listbox.c
 $(CC) $(C_SW)-fo=$*.Obj listbox.c

Listbox.Res: Listbox.Rc
 $(RC) $(RC_SW) Listbox.Rc $*.Res

listmgr.Obj: listmgr.c
 $(CC) $(C_SW)-fo=$*.Obj listmgr.c

SndMgr.Obj: SndMgr.C
 $(CC) $(C_SW)-fo=$*.Obj SndMgr.C

TraffMgr.Obj: TraffMgr.C
 $(CC) $(C_SW)-fo=$*.Obj TraffMgr.C

EntryFld.Obj: EntryFld.C
 $(CC) $(C_SW)-fo=$*.Obj EntryFld.C

