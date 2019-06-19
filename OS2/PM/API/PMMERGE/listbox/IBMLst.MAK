
# MAKE file for IBMLst.MAK     Version 1.1

# Revised:  1994-04-20

# Copyright ¸ 1987-1994  Prominare Inc.

# MAKE file created by Prominare Builder  Version 2.1b

# Macro definitions

CC=ICC
RC=RC
# C_SW=-G3e- -O- -Oi- -Op- -Os- -Ti -Rn -W3 -C
C_SW=-G4e- -O+ -Rn -W3 -C
# L_SW=/DEBUG /NOE
L_SW=/NOE
RC_SW=-r

ListBox.Dll: EventMgr.Obj heapmgr.Obj\
             listbox.Obj listmgr.Obj\
             SndMgr.Obj TraffMgr.Obj\
             EntryFld.Obj listbox.Res
 Link386 $(L_SW) @IBMList.Lnk;
 rc -x Listbox.Res listbox.dll

EventMgr.Obj: EventMgr.C
 $(CC) $(C_SW) -Fo$*.Obj EventMgr.C

heapmgr.Obj: heapmgr.c
 $(CC) $(C_SW) -Fo$*.Obj heapmgr.c

listbox.Obj: listbox.c
 $(CC) $(C_SW) -Fo$*.Obj listbox.c

Listbox.Res: Listbox.Rc
 $(RC) $(RC_SW) Listbox.Rc $*.Res

listmgr.Obj: listmgr.c
 $(CC) $(C_SW) -Fo$*.Obj listmgr.c

SndMgr.Obj: SndMgr.C
 $(CC) $(C_SW) -Fo$*.Obj SndMgr.C

TraffMgr.Obj: TraffMgr.C
 $(CC) $(C_SW) -Fo$*.Obj TraffMgr.C

EntryFld.Obj: EntryFld.C
 $(CC) $(C_SW) -Fo$*.Obj EntryFld.C

