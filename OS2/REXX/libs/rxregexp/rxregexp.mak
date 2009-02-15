#/*----------------------------------------------------------------------------*/
#/*                                                                            */
#/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
#/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
#/*                                                                            */
#/* This program and the accompanying materials are made available under       */
#/* the terms of the Common Public License v1.0 which accompanies this         */
#/* distribution. A copy is also available at the following address:           */
#/* http://www.oorexx.org/license.html                          */
#/*                                                                            */
#/* Redistribution and use in source and binary forms, with or                 */
#/* without modification, are permitted provided that the following            */
#/* conditions are met:                                                        */
#/*                                                                            */
#/* Redistributions of source code must retain the above copyright             */
#/* notice, this list of conditions and the following disclaimer.              */
#/* Redistributions in binary form must reproduce the above copyright          */
#/* notice, this list of conditions and the following disclaimer in            */
#/* the documentation and/or other materials provided with the distribution.   */
#/*                                                                            */
#/* Neither the name of Rexx Language Association nor the names                */
#/* of its contributors may be used to endorse or promote products             */
#/* derived from this software without specific prior written permission.      */
#/*                                                                            */
#/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS        */
#/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          */
#/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS          */
#/* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   */
#/* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      */
#/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
#/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,        */
#/* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY     */
#/* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING    */
#/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         */
#/* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               */
#/*                                                                            */
#/*----------------------------------------------------------------------------*/
#------------------------
# RXREGEXP.MAK make file
#------------------------
#all: $(OR_OUTDIR)\rxregexp.dll \
#     $(OR_OUTDIR)\rxregexp.cmd
all: $(OR_OUTDIR)\rxregexp.dll
    @ECHO .
    @ECHO All done RXREGEXP.DLL
    @ECHO .

!include "..\LIB\ORXWIN32.MAK"

!IFNDEF OR_ORYXREGEXP
!ERROR Build error, OR_ORYXREGEXP not set
!ENDIF

CPPOBJS = $(OR_OUTDIR)\rxregexp.obj $(OR_OUTDIR)\automaton.obj $(OR_OUTDIR)\dblqueue.obj

# Following for RXREGEXP.LIB
#
# *** RXREGEXP.LIB  : Creates .lib import library
#                          .exp export library for use with this link
#
# Generate import library (.lib) and export library (.exp) from
# module-definition (.dfw) file for a DLL
$(OR_OUTDIR)\RXREGEXP.lib : $(CPPOBJS) $(OR_ORYXREGEXP)\RXREGEXP.def
        $(OR_IMPLIB) -machine:$(CPU) \
        -def:$(OR_ORYXREGEXP)\RXREGEXP.def \
        $(CPPOBJS)               \
        -out:$(OR_OUTDIR)\RXREGEXP.lib

#
# *** RXREGEXP.DLL
#
# need import libraries and def files still
$(OR_OUTDIR)\rxregexp.dll : $(CPPOBJS) $(OBJS) $(RXDBG_OBJ) $(OR_OUTDIR)\RXREGEXP.lib \
                            $(OR_ORYXREGEXP)\RXREGEXP.def     \
                            $(OR_OUTDIR)\RXREGEXP.exp
    $(OR_LINK) -map $(lflags_common) $(lflags_dll) -out:$(OR_OUTDIR)\$(@B).dll \
             $(CPPOBJS) $(RXDBG_OBJ) \
             $(OR_OUTDIR)\verinfo.res \
             $(OR_OUTDIR)\$(@B).exp \
             $(OR_OUTDIR)\rexx.lib \
             $(OR_OUTDIR)\rexxapi.lib \
             $(libs_dll)

#
# *** .cpp -> .obj rules
#
$(CPPOBJS):  $(@B).cpp
    @ECHO .
    @ECHO Compiling $(@B).cpp
    $(OR_CC) $(cflags_common) /GX $(cflags_dll) /Fo$(OR_OUTDIR)\$(@B).obj $(OR_ORYXINCL) $(OR_ORYXREGEXP)\$(@B).cpp
