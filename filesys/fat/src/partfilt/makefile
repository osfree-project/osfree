# $Id: makefile 2 2002-01-07 00:00:03Z ktk $ 

#
# $Header$
#
# This is the MS Visual C++ version of ext2flt.flt makefile. I "ported"
# Deon's filter driver to MSC simply because I don't have Borland C++
# to compile ext2flt.flt !! Normally I didn't hack the code too much, the
# main differences are :
#     - segment definitions (CONST segment) in asm files, and a new linker
#       definition file to take these into account (ext2flt2.asm) have been
#       changed to meet MSC requirements.
#     - memcpy and memset routines in e2wrap.asm are no longer used, but
#       the builtin MSC version are used instead (MSC inline them). I did this
#       because of some obscure conflicts with MSC memory models. This change
#       should be safe because I use builtin memset and memcpy routines in
#       ext2-os2.ifs for a year now :-))
#     - macros ENABLE and DISABLE, which use inline assembly, have been replaced
#       with MSC builtin _enable() and disable() functions ... just to avoid
#       compiler warnings about inline asm and optimization (the code produced
#       is the same).
#
# These differences shouldn't really affect the way ext2flt.flt works, but in
# case you encounter a problem, please try to reproduce it with Deon's original
# Borland C++ compiled version and please drop me a note if it does not
# behave the same way. Deon's original package can be found either on
# ftp-os2.nmsu.edu (/os2/diskutil/e2flt12c.zip) or on quark.cs.sun.ac.za
# (/pub/ext2flt/e2flt12c.zip)
#
# The __MSC__ macro is used for MS Visual C++ specific stuff
# The __MASM__ macro is used for MASM stuff (MASM 5.51 from Devcon DDK 2.0)
#
# Matthieu WILLM, 1995-12-1
#

#!include ..\makefile.inc
#
# HK: Modified since my location is elsewhere
#
DDKPATH=E:\OS2DDK\DDK

# Define targets to build
all:    partfilt.flt

debug:  partfilt.flt

# MS Visual C++ 1.51 compiler options
#       -Answ : small memory model, DS != SS and other goodies !
#       -G3 : compile for 80386
#       -O2 : optimise code
#       -On : disable unsafe optimisation
#       -Gs : no stack probes
#       -Lp : protected mode code
#       -Zp : pack structures on one byte boundaries
CC_OPT= -Answ -O2 -On -G2s -Lp -Zp -D__MSC__ -I.  /Fc%|dpfF.lst
AS_OPT= -c -Cp -D__MASM__ -I$(DDKPATH)\base\inc /W3
LN_OPT= /map /noe /nopackcode

# Include path for files
INCLUDE=$(INCLUDE);(DDKPATH)\base\h;(DDKPATH)\base\src\dev\dasd\diskh
INC= include


# Names of complier, assembler and linker.
CC = cl -c $(CC_OPT) 
AS = ml $(AS_OPT)
LINK = link

# Implicit rules
.c.obj:
        $(CC) $< >$*.err
        type $*.err

.asm.obj:
        $(AS) $< >$*.err
        type $*.err

INCS=debug.h e2data.h e2filt.h e2init.h e2inutil.h e2iocmd.h e2part.h \
     e2router.h e2virtio.h e2wrap.h os2head.h  

# Define object files used to build executable from:
OBJ_1= e2header.obj e2wrap.obj e2data.obj
OBJ_2= e2router.obj e2filt.obj e2part.obj
OBJ_3= e2iocmd.obj e2virtio.obj e2init.obj
OBJ_4= e2inutil.obj

OBJS= $(OBJ_1) $(OBJ_2) $(OBJ_3) $(OBJ_4)

# Make sure that the new debug flag is active...
debug.h: MAKEFILE

# List macros
EXE_depend = $(OBJS)

# Explicit rules
partfilt.flt: $(EXE_depend) partfilt.def MAKEFILE
        $(LINK) $(LN_OPT) @<<
$(OBJ_1) +
$(OBJ_2) +
$(OBJ_3) +
$(OBJ_4)
partfilt.flt
partfilt.map
/nod:slibce slibcep os2
partfilt.def
<<
#        mapsym partfilt.map

install:
        mapsym partfilt.map
        copy partfilt.flt e:\mwfs
        copy partfilt.sym e:\mwfs

clean:
        -del $(OBJS)
        -del partfilt.flt
        -del partfilt.sym
        -del partfilt.map
        -del *.l
        -del *.lst

e2header.obj : e2header.asm
e2wrap.obj   : e2wrap.asm


e2data.obj   : e2data.c   $(INCS)
e2filt.obj   : e2filt.c   $(INCS)   
e2init.obj   : e2init.c   $(INCS)   
e2inutil.obj : e2inutil.c $(INCS)  
e2iocmd.obj  : e2iocmd.c  $(INCS)   
e2part.obj   : e2part.c   $(INCS)   
e2router.obj : e2router.c $(INCS)  
e2virtio.obj : e2virtio.c $(INCS)  

