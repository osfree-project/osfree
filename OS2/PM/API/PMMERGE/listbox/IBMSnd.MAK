
# MAKE file for IBMSnd.MAK     Version 1.1

# Revised:  1994-04-20

# Copyright ¸ 1987-1994  Prominare Inc.

# MAKE file created by Prominare Builder  Version 2.1b

# Macro definitions

CC=ICC
RC=RC
# C_SW=-G3e- -O- -Oi- -Op- -Os- -Ti -Rn -W3 -C
C_SW=-G4e- -O+ -Rn -W3 -C
RC_SW=-r 

LBSnd.Dll: LBSnd.Obj
 Link386 /DEBUG @IBMSnd.Lnk;

LBSnd.Obj: LBSnd.C
 $(CC) $(C_SW) -Fo$*.Obj LBSnd.C
