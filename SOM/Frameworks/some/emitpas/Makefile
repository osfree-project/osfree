SRCS        = emitpas.c pas.c sequences.c strpas.c pas.efw pas.idl
OBJS        = emitpas.obj pas.obj sequences.obj strpas.obj
TARGET      = emitpas.dll
CLEANFILES  = pas.h

CC          = wcc386
CFLAGS      = -bd -bm -bt=OS2 -5 -fpi -sg -otexan -wx -wcd=1177
LINKER      = wlink
#LIBLIST     = somtk os2386

.SUFFIXES:
.SUFFIXES: .obj .ih .idl .c

.c.obj:
  $(CC) $(CFLAGS) $<

$(TARGET): $(OBJS)
  $(LINKER) @emit.lnk
clean: .symbolic
  @if exist *.obj del *.obj
  @if exist emitpas.dll del emitpas.dll

strpas.o: strpas.c strpas.h

sequences.o: sequences.c sequences.h

emitpas.o: emitpas.c pas.ih

pas.o: pas.c pas.ih
