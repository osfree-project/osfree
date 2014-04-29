CXX=wpp386
CC=wcc386
LNK=wlink
MAPCNV=..\mapsym.awk

COPT=-3s -s -hw -mf -od -zq -q -i=. -i=.. -i=..\include -i=$(%WATCOM)\h -i=$(%WATCOM)\h\os2

OBJS1=f32stat.obj
OBJS2=cachef32.obj
OBJS3=monitor.obj
OBJS4=diskinf0.obj diskinf1.obj diskinf2.obj

TARGETS=f32stat.exe monitor.exe diskinfo.exe  &
        cachef32.exe diskdump.exe diskdump.sym cachef32.sym &
        diskinfo.sym monitor.sym f32stat.sym

SYS=os2v2

.SUFFIXES:
.SUFFIXES: .dll .exe .lnk .ols .obh .obj .cpp .c .h .sym .map .wmp

all: $(TARGETS)

#diskinf0.c: fat32def.h fat32.h
#diskinf1.c: fat32def.h fat32.h
#diskinf2.c: fat32def.h fat32.h
#monitor.c:  fat32def.h
#cachef32.c: fat32def.h
#f32stat.c:  fat32def.h

f32stat.ols: $(OBJS1)
 @%create $^@
 @for %e in ($<) do @%append $^@ FILE %e

cachef32.ols: $(OBJS2)
 @%create $^@
 @for %e in ($<) do @%append $^@ FILE %e

monitor.ols: $(OBJS3)
 @%create $^@
 @for %e in ($<) do @%append $^@ FILE %e

diskinfo.ols: $(OBJS4)
 @%create $^@
 @for %e in ($<) do @%append $^@ FILE %e

diskdump.ols: $(OBJS4)
 @%create $^@
 @for %e in ($<) do @%append $^@ FILE %e

.ols.lnk:
 @%create $^@
 @%append $^@ SYSTEM $(SYS)
 @%append $^@ NAME $^*.exe
 @%append $^@ OPTION DESCRIPTION 'UFAT32 Helper DLL, Henk Kelder'
 @%append $^@ OPTION ST=81920
 @%append $^@ OPTION QUIET
 @%append $^@ DEBUG ALL
 @%append $^@ OPTION MAP=$^*.wmp
 @%append $^@ LIBPATH $(%WATCOM)\lib386;$(%WATCOM)\lib386\os2;
 @%append $^@ LIB os2386,clib3s
 @%append $^@ @$[@

.lnk.exe:
 $(LNK) @$[@

.c.obj:
 $(CC) $(COPT) $[@

.cpp.obj:
 $(CXX) $(COPT) $[@

.wmp.map:
 awk -f $(MAPCNV) $< >$^@

.map.sym:
 @mapsym $[@ >nul 2>&1
