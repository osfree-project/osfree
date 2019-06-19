
# MAKE file for BCLst.MAK     Version 1.1

# Revised:  1994-04-21

# Copyright ¸ 1987-1994  Prominare Inc.

# MAKE file created by Prominare Builder  Version 2.1b

# Macro definitions

CC=BCC
RC=Rc
C_SW=-sd -Ox -v -w -c -IJ:\BCOS2\INCLUDE;D:\Include;N:\MMOS2\MMTOOLKT\H;
RC_SW=-r
LNK_SW=-v -Tod -A:2 -LJ:\BCOS2\LIB;


ListBox.Dll: EventMgr.Obj heapmgr.Obj listbox.Obj\
        Listbox.Res listmgr.Obj SndMgr.Obj\
        TraffMgr.Obj EntryFld.Obj
 Tlink $(LNK_SW) @BCLst.Lnk;
 rc ListBox.res ListBox.Dll

EventMgr.Obj: EventMgr.C
 $(CC) $(C_SW)-o$*.Obj EventMgr.C

heapmgr.Obj: heapmgr.c
 $(CC) $(C_SW)-o$*.Obj heapmgr.c

listbox.Obj: listbox.c
 $(CC) $(C_SW)-o$*.Obj listbox.c

Listbox.Res: Listbox.Rc
 $(RC) $(RC_SW) Listbox.Rc $*.Res

listmgr.Obj: listmgr.c
 $(CC) $(C_SW)-o$*.Obj listmgr.c

SndMgr.Obj: SndMgr.C
 $(CC) $(C_SW)-o$*.Obj SndMgr.C

TraffMgr.Obj: TraffMgr.C
 $(CC) $(C_SW)-o$*.Obj TraffMgr.C

EntryFld.Obj: EntryFld.C
 $(CC) $(C_SW)-o$*.Obj EntryFld.C

