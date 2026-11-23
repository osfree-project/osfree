
# MAKE file for IBMDvr.MAK     Version 1.1

# Revised:  1994-07-03

# Copyright ¸ 1987-1994  Prominare Inc.

# MAKE file created by Prominare Builder  Version 2.1b

# Macro definitions

CC=ICC
# C_SW=-G3 -O- -Oi- -Op- -Os- -Ti -W3 -C
#L_SW=/DEBUG
C_SW=-G3 -O+ -W3 -C
L_SW=


Driver.Exe: Driver.Obj
 Link386 $(L_SW) @IBMDvr.Lnk;

Driver.Obj: Driver.C
 $(CC) $(C_SW) -Fo$*.Obj Driver.C

