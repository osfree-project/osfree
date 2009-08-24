
# MAKE file for BCSnd.MAK     Version 1.1

# Revised:  1994-04-21

# Copyright ¸ 1987-1994  Prominare Inc.

# MAKE file created by Prominare Builder  Version 2.1b

# Macro definitions

CC=BCC
C_SW=-sd -Ox -v -w -c -IJ:\BCOS2\INCLUDE;D:\Include;N:\MMOS2\MMTOOLKT\H;
LNK_SW=-v -Tod -A:2 -LJ:\BCOS2\LIB;N:\MMOS2\MMTOOLKT\LIB;


LbSnd.Dll: LBSnd.Obj
 Tlink $(LNK_SW) @BCSnd.Lnk;

LBSnd.Obj: LBSnd.C
 $(CC) $(C_SW)-o$*.Obj LBSnd.C

