# ------------------------------------------------------------------------
#  Makefile for string library used in dskxtrct v1.2 and others
#  Works for IBM Visual Age C++ version 3.6.5.  ilib invocation will
#  need to be changed for VAC++ 3.0.8,
#
#     Jon Saxton, January 2001
#
# This makefile is part of the dskxtrct source code and is distributed
# under the same GNU General Public Licence.
#
# ------------------------------------------------------------------------
#
# Object and other intermediate files are put into an obj subdirectory.
# Library is constructed in the parent directory.
#
# ------------------------------------------------------------------------

OBJ = obj

.SUFFIXES:

.SUFFIXES: \
    .c .obj

.c{obj}.obj:
    icc.exe -Q -Ss -Ti- -O+ -C /Fo$@ %s

..\stringe.lib : $(OBJ)\stringe.obj
       -@del $@ >nul 2>&1
# for Visual Age C++ version 3.6.5
#       ilib /out:$@ $**
# for Visual Age C++ version 3.0.8
        ilib /nologo /nobrowse /nobackup $@ -+$**;

# DO NOT DELETE THIS LINE -- make depend depends on it.

$(OBJ)\stringe.obj: stringe.c
