.AUTODEPEND

CFG_DEPENDENCIES = utils.mak

!include "..\config.mak"

all: $(CFG) mktools.exe mkctxt.exe chunk.exe mkinfres$(EXE_SUF)

mktools.c : ..\config.h
