
# not used currently.
# goal is to implement a host which works with DR-DOS EMM386

# creates HDPMI32.EXE. use: "nmake /f HDPMI32.MAK"
# tools used:
#  - JWasm | Masm
#  - WLib (Open Watcom) | MS OMF LIB (lib16.exe)
#  - Digital Mars OMF linker (link.exe)

!include <..\dirs>

!ifndef DEBUG
DEBUG = 0
!endif

!if $(DEBUG)
ASMD=-Zi -D_DEBUG
LINKD=/LI
!endif

SRCMODS = \
!include modules.inc
OBJNAMES = $(SRCMODS:.ASM=.OBJ)
!if $(DEBUG)
CP = 0
OBJMODS = $(OBJNAMES:.\=DRDOS32\)
!else
CP = 1
OBJMODS = $(OBJNAMES:.\=DRDOS32\)
!endif

LIB16 = wlib.exe -q
#LINK  = link16.exe
LINK  = $(DMCDIR)\link.exe
AS    = @jwasm.exe
AFLAGS= -nologo -c -Sg -D?32BIT=1 -D?MPICBASE=50h $(ASMD) -I$(INC32DIR) -Fl$* -Fo$*
NAME  = HDPMI32
SRC   = hdpmi
LOPTS = /NOLOGO/FAR/MAP:FULL/NOE/NON/ONERROR:NOEXE/ST:512 $(LINKD)

DEPS0 = hdpmi.inc external.inc

!if $(DEBUG)
OUTDIR=DRDOS32D
!else
OUTDIR=DRDOS32
!endif

.SUFFIXES: .asm .obj

.asm{$(OUTDIR)}.obj:
    $(AS) $(AFLAGS) $<

ALL: $(OUTDIR)\$(NAME).EXE

$(OUTDIR)\$(NAME).EXE: $(OUTDIR)\$(SRC).obj $(OUTDIR)\$(NAME).lib $(NAME).mak
    $(LINK) @<<
$(OUTDIR)\$(SRC).obj $(LOPTS),
$*.EXE,
$*.MAP,
$(OUTDIR)\$(NAME);
<<
!if $(CP)
#     copy $*.EXE ..\..\BIN\*.*
#!ifdef TOOLSDIR
#    copy $*.EXE $(TOOLSDIR)\*.*
#!endif
!endif     

$(OUTDIR)\$(NAME).lib: $(OBJMODS) $(NAME).mak
	@cd $(OUTDIR)
	@erase $(NAME).lib
	$(LIB16) -n $(NAME).lib $(OBJNAMES:.\=+)
    @cd ..

$(OBJMODS): hdpmi.inc external.inc version.inc
