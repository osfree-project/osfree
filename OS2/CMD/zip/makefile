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
ADD_COPT = -dOS2 -dNO_ASM -dNO_SYMLINKS &
           -zq -Ox -s -Zp1 -i=$(MYDIR)$(SEP)os2 -d3 # -sg -ecw 
ADD_ASMOPT = -zq -3p
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
UNI2H = 1
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe
# $(p)crctab$(e)

!include $(%ROOT)tools/mk/appsos2_cmd.mk
