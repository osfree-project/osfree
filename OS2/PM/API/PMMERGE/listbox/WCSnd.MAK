
# MAKE file for WCSnd.MAK     Version 1.1

# Revised:  1994-04-21

# Copyright ¸ 1987-1994  Prominare Inc.

# MAKE file created by Prominare Builder  Version 3.0

# Macro definitions

CC=Wcl386
C_SW=-c -ms -3s -bd -s -od -d2 -zp4 -w3 


LBSnd.dll: lbsnd.Obj
 WLink DEBUG ALL @WCSnd.LNK

lbsnd.Obj: lbsnd.c
 $(CC) $(C_SW)-fo=$*.Obj lbsnd.c

