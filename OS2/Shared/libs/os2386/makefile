#
#
#

PROJ = os2386
TRGT = $(PROJ).lib
DIRS = libc
lbcs =  bdcalls.lbc doscalls.lbc helpmgr.lbc sub32.lbc  &
#       kbdcalls.lbc moncalls.lbc moucalls.lbc viocalls.lbc &
        msg.lbc nampipes.lbc nls.lbc pmctls.lbc pmdrag.lbc pmgpi.lbc &
        pmspl.lbc pmshapi.lbc pmviop.lbc pmwin.lbc pmwp.lbc quecalls.lbc sesmgr.lbc
UNI2H = 1

!ifndef TARGETBLD
DEST = host$(SEP)$(%HOST)$(SEP)os2tk45$(SEP)lib
SRC  = $(MYDIR)
!else
DEST = os2tk45$(SEP)lib
SRC  = $(MYDIR)..$(SEP)..$(SEP)os2386$(SEP)
!endif

#TARGETS  = $(PATH)$(PROJ).lib subdirs

!include $(%ROOT)tools/mk/libsos2.mk

.SUFFIXES:
.SUFFIXES: .lib .lbc .def .obj .asm

LOPT = -q -n
#DEST = $(BLD)lib

.lib: $(PATH)

.lbc: $(PATH)

.obj: $(PATH)

.def: $(MYDIR)

.def: $(SRC)

.asm: $(MYDIR)

.asm: $(SRC)

.asm.obj: .AUTODEPEND
 @$(SAY) ASM      $^. $(LOG)
 @$(ASM) $(ASMOPT) -fr=$^*.err -fo=$^@ $[@ $(LOG2)

$(PATH)os2386.lbc: $(lbcs)
        @%create $@
!ifeq UNIX TRUE
        @for %f in ($<) do @cat %f >>$@
!else
        @for %f in ($<) do @type %f >>$@
!endif

.def.lbc:
        @sed -f $(SRC)def2lbc.sed $[@ >$(PATH)$^@

$(PATH)$(PROJ).lib: $(PATH)os2386.lbc $(PATH)msgseg32.obj
        @$(LIB) $(LOPT) $@ @$[@ +$]@ $(LOG2)
