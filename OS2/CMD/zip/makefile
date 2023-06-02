#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ = zip
TRGT = $(PROJ).exe
DESC = ZIP file archiver
# defines additional options for C compiler
# ADD_LINKOPT = lib cmd_shared.lib,all_shared.lib
DIRSEP="\\"
AS_DIRSEP="\\"
ADD_COPT += -dOS2 -dNO_ASM -dNO_SYMLINKS -zq -ox -zp=1 -s -mf -d3 # -sg -ecw
ADD_ASMOPT += -zq -3p -mf
ADD_LINKOPT = debug all # op caseexact
STACK=0x50000
srcfiles = $(p)zip$(e) $(p)zipfile$(e) $(p)zipup$(e) $(p)fileio$(e) $(p)util$(e) &
           $(p)crc32$(e) $(p)globals$(e) $(p)match32$(e) &
           $(p)deflate$(e) $(p)trees$(e) $(p)crypt$(e) $(p)ttyio$(e) &
           $(p)os2zip$(e) $(p)os2$(e) $(p)os2acl$(e)
ALIASES = &
           window=_window, &
           prev=_prev, &
           prev_length=_prev_length, &
           strstart=_strstart, &
           match_start=_match_start, &
           max_chain_length=_max_chain_length, &
           good_match=_good_match, &
           nice_match=_nice_match
NODEFAULTLIBS = 1
UNI2H = 1
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe

!include $(%ROOT)tools/mk/appsos2_cmd.mk
!include $(MYDIR)zip.mk

ADD_COPT += -i=$(PORT_BASE) -i=$(PORT_BASE)os2

.asm: $(PORT_BASE)os2

.c: $(PORT_BASE)os2;$(PORT_BASE)

.h: $(PORT_BASE)os2;$(PORT_BASE)
