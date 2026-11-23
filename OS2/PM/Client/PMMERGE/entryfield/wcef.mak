
# MAKE file for WCEF.MAK     Version 1.1

# Revised:  1994-07-03

# Copyright ¸ 1987-1994  Prominare Inc.

# MAKE file created by Prominare Builder  Version 2.1b

# Macro definitions

CC=Wcl386
RC=rc
C_SW=-c -ms -3s -bd -s -od -d2 -zp4 -w3
RC_SW=-r


EntryFld.Dll: EntryFld.Obj HeapMgr.Obj EventMgr.Obj
 WLink DEBUG ALL @WCEF.LNK

EntryFld.Obj: EntryFld.C
 $(CC) $(C_SW)-fo=$*.Obj EntryFld.C

HeapMgr.Obj: HeapMgr.C
 $(CC) $(C_SW)-fo=$*.Obj HeapMgr.C

EventMgr.Obj: EventMgr.C
 $(CC) $(C_SW)-fo=$*.Obj EventMgr.C

