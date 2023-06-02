#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ = unzip
TRGT = $(PROJ).exe
DESC = UNZIP file unarchiver
# defines additional options for C compiler
# ADD_LINKOPT = lib cmd_shared.lib,all_shared.lib
DIRSEP="\\"
AS_DIRSEP="\\"
ADD_COPT += -dOS2 -dASM_CRC -dUSE_OWN_CRCTAB -zq -ox -s -zp=1 -mf -d3 # -sg -ecw
ADD_ASMOPT += -dOS2 -dASM_CRC -dUSE_OWN_CRCTAB -zq -3p -mf

ADD_LINKOPT = debug all # op caseexact
STACK=0x50000
srcfiles = $(p)unzip$(e) $(p)crypt$(e) $(p)envargs$(e) $(p)explode$(e) $(p)extract$(e) &
           $(p)crc32$(e) $(p)fileio$(e) $(p)globals$(e) &
           $(p)inflate$(e) $(p)list$(e) $(p)match$(e) $(p)process$(e) $(p)ttyio$(e) &
           $(p)ubz2err$(e) $(p)unreduce$(e) $(p)unshrink$(e) $(p)zipinfo$(e) &
           $(p)os2$(e) $(p)os2acl$(e) $(p)crc_i386$(e)
#ALIASES = &
#           window=_window, &
#           prev=_prev, &
#           prev_length=_prev_length, &
#           strstart=_strstart, &
#           match_start=_match_start, &
#           max_chain_length=_max_chain_length, &
#           good_match=_good_match, &
#           nice_match=_nice_match
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe
NODEFAULTLIBS = 1
UNI2H = 1

!include $(%ROOT)tools/mk/appsos2_cmd.mk
!include $(MYDIR)unzip.mk

ADD_COPT += -i=$(PORT_BASE)os2 -i=$(PORT_BASE)
ADD_ASMOPT += -i=$(PORT_BASE)os2 -i=$(PORT_BASE)

.asm: $(PORT_BASE)os2

.c: $(PORT_BASE)os2;$(PORT_BASE)

.h: $(PORT_BASE)os2;$(PORT_BASE)
