LIB=wlib
AS=wasm
CC=wcc

COPT=-2 -ml -ecw -r -s -zdp -zff -zgf -zls -zp=1 -zt -zu -zl -ze -zq -od -of+ -q -d__16BITS__ -d__WATCOM -i=. -i=.. -i=..\include
AOPT=-q -d__LARGE__ -i=. -i=.. -i=..\include
OBJS=mem.obj i4d.obj i4m.obj pia.obj pts.obj ptc.obj alphabet.obj
TARGETS=libc.lib

.SUFFIXES:
.SUFFIXES: .lib .ols .obj .c .h .asm

all: $(TARGETS)

libc.ols: $(OBJS)
 @%create $^@
 @for %e in ($<) do @%append $^@ +%e

.c.obj:
 $(CC) $(COPT) $[@

.asm.obj:
 $(AS) $(AOPT) $[@

.ols.lib:
 $(LIB) $^@ @$<
