
# MAKE file for IBMEF.MAK     Version 1.1

# Revised:  1994-07-03

# Copyright ¸ 1987-1994  Prominare Inc.

# MAKE file created by Prominare Builder  Version 2.1b

# Macro definitions

CC=ICC
RC=RC
#C_SW=-G3e- -O- -Oi- -Op- -Os- -Ti -Rn -W3 -C
#L_SW=/DEBUG
C_SW=-G3e- -O+ -Rn -W3 -C
L_SW=
RC_SW=-r

EntryFld.Dll: EntryFld.Obj HeapMgr.Obj EventMgr.Obj
 Link386 $(L_SW) @IBMEf.Lnk;

EntryFld.Obj: EntryFld.C
 $(CC) $(C_SW) -Fo$*.Obj EntryFld.C

HeapMgr.Obj: HeapMgr.C
 $(CC) $(C_SW) -Fo$*.Obj HeapMgr.C

EventMgr.Obj: EventMgr.C
 $(CC) $(C_SW) -Fo$*.Obj EventMgr.C

