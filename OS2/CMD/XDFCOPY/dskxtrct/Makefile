# ------------------------------------------------------------------------
#     Makefile for dskxtrct v1.2 and others
#     Works for IBM Visual Age C++ version 3.6.5 and probably 3.0.8
#
#     Jon Saxton, January 2001
#
# This makefile is part of the dskxtrct source code and is distributed
# under the same GNU General Public Licence.
#
# ------------------------------------------------------------------------
#
# Object and other intermediate files are put into an obj subdirectory.
# Run-time files and documentation appear in the parent directory.
#
# ------------------------------------------------------------------------

OBJ = obj

.SUFFIXES:

.SUFFIXES: .cpp .obj

.cpp{$(OBJ)}.obj:
       icc.exe /I../dskinc /I../stringe /Ti- /O+ /C /Fo$@ $<

all:   ..\dskxtrct.exe

../dskxtrct.exe: $(OBJ)/dskxtrct.obj ../stringe.lib
       icc /B"/de /optfunc" /Fe$@ $**

# DO NOT DELETE THIS LINE -- make depend depends on it.

$(OBJ)/dskxtrct.obj: dskxtrct.cpp printmsg.i ../stringe/stringe.h initial0.i
$(OBJ)/dskxtrct.obj: initial2.i initial.i dsklogic.i
