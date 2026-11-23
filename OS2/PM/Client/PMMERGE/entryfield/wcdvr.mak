
# MAKE file for WCDvr.MAK     Version 1.1

# Revised:  1994-07-03

# Copyright ¸ 1987-1994  Prominare Inc.

# MAKE file created by Prominare Builder  Version 2.1b

# Macro definitions

CC=Wcl386
C_SW=-c -ms -3s -s -od -d2 -zp4 -w3 


Driver.Exe: Driver.Obj
 WLink DEBUG ALL @WCDvr.LNK

Driver.Obj: Driver.C
 $(CC) $(C_SW)-fo=$*.Obj Driver.C

