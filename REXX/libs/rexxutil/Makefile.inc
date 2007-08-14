# General Unix Makefile for regutil
#
# You are meant to set the P variables (PDEBUG et al) in a platform-
# specific Makefile, then include Makefile.inc from that other file

#  The contents of this file are subject to the Mozilla Public License
#  Version 1.0 (the "License"); you may not use this file except in
#  compliance with the License. You may obtain a copy of the License at
#  http://www.mozilla.org/MPL/

#  Software distributed under the License is distributed on an "AS IS"
#  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
#  License for the specific language governing rights and limitations
#  under the License.

#  The Original Code is regutil.

#  The Initial Developer of the Original Code is Patrick TJ McPhee.
#  Portions created by Patrick McPhee are Copyright © 1998, 2001
#  Patrick TJ McPhee. All Rights Reserved.

#  Contributors:

# $Header: /netlabs.cvs/osfree/src/REXX/libs/rexxutil/Makefile.inc,v 1.3 2004/08/21 14:48:37 prokushev Exp $

DEBUG=-g $(PDEBUG)
OPT=

REXX_INCLUDE=$(HOME)/regina
CFLAGS=$(DEBUG) $(OPT) $(PCFLAGS) $(PCDEFS) -I $(REXX_INCLUDE)
LDFLAGS=$(PLDFLAGS)
LIBS=$(PLIBS)
LIBFILE=librexxutil
so=so

# object files
OBJS=regfilesys.o regini.o regmacrospace.o regscreenux.o regsemux.o\
 regutil.o rxsupport.o regstem.o inifile.o regunicode.o regcrypt.o

all : $(LIBFILE).$(so)

$(LIBFILE).$(so) : $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(LIBS) -o $(LIBFILE).$(so)

$(OBJS) :: regutil.h rxproto.h

clean:
	-rm *.o $(LIBFILE).$(so)

