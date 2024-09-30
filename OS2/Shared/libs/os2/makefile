#
#
#

PROJ = os2
TRGT = $(PROJ).lib
DIRS = libc
lbcs =  ansicall.lbc doscalls.lbc &
        kbdcalls.lbc moncalls.lbc moucalls.lbc &
        msg.lbc nampipes.lbc nls.lbc pmgpi.lbc &
        pmshapi.lbc pmviop.lbc pmwin.lbc quecalls.lbc sesmgr.lbc viocalls.lbc
UNI2H = 1

!ifndef TARGETBLD
DEST = host$(SEP)$(%HOST)$(SEP)os2tk45$(SEP)lib
SRC  = $(MYDIR)
!else
DEST = os2tk45$(SEP)lib
SRC  = $(MYDIR)..$(SEP)..$(SEP)os2$(SEP)
!endif


!include $(%ROOT)tools/mk/libsos2v1.mk

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

$(PATH)os2.lbc: $(lbcs)
        @%create $@
!ifeq UNIX TRUE
        @for %f in ($<) do @cat %f >>$@
!else
        @for %f in ($<) do @type %f >>$@
!endif

.def.lbc:
        @sed -f $(SRC)def2lbc.sed $[@ >$(PATH)$^@

$(PATH)$(PROJ).lib: $(PATH)os2.lbc $(PATH)msgseg.obj
        @$(LIB) $(LOPT) $@ @$[@ +$]@ $(LOG2)
