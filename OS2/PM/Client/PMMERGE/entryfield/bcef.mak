
# MAKE file for BCEF.MAK     Version 1.0

# Revised:  1994-07-03

# Copyright ¸ 1987-1994  Prominare Inc.

# MAKE file created by Prominare Builder  Version 2.1b

# Macro definitions

CC=BCC
RC=Rc
C_SW=-sd -Ox -v -w -c -IJ:\BCOS2\INCLUDE
RC_SW=-r
LNK_SW=-v -Tod -A:2 -LJ:\BCOS2\LIB;


EntryFld.Dll: EntryFld.Obj HeapMgr.Obj EventMgr.Obj
 Tlink $(LNK_SW) @BCEF.Lnk;

EntryFld.Obj: EntryFld.C
 $(CC) $(C_SW)-o$*.Obj EntryFld.C

HeapMgr.Obj: HeapMgr.C
 $(CC) $(C_SW)-o$*.Obj HeapMgr.C

EventMgr.Obj: EventMgr.C
 $(CC) $(C_SW)-o$*.Obj EventMgr.C


