
# MAKE file for Driver.MAK     Version 3.0

# Revised:  1994-04-21

# Copyright ¸ 1987-1994  Prominare Inc.

# MAKE file created by Prominare Builder  Version 3.0

# Macro definitions

OBJDIR=Exe
AS=ml
BC=
CC=ICC
CPP=
FOR=
PAS=
RC=Rc
SC=SC
ASM_SW=-c -FPi -Cx -Zi -W3 
BAS_SW=
C_SW=-G3s- -O- -Oi- -Op- -Os- -Ti -W3 -C 
CPP_SW=
FOR_SW=
PAS_SW=
RC_SW=-r 
SC_SW=-w -a cstyle=s -a ibmc


Update: $(OBJDIR)Driver.Obj

$(OBJDIR)Driver.Obj: Driver.C
 $(CC) $(C_SW)-Fo$*.Obj Driver.C

