#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ = zip
DESC = ZIP file archiver
# defines additional options for C compiler
ADD_LINKOPT = lib cmd_shared.lib,all_shared.lib
ADD_COPT = -i=..$(SEP)include -DOS2 -DNO_ASM -DNO_SYMLINK
srcfiles = $(p)zip$(e) $(p)zipfile$(e) $(p)zipup$(e) $(p)fileio$(e) $(p)util$(e) &
           $(p)crc32$(e) $(p)crctab$(e) $(p)globals$(e) &
           $(p)deflate$(e) $(p)trees$(e) $(p)crypt$(e) $(p)ttyio$(e) &
           $(p)os2zip$(e) $(p)os2$(e) $(p)os2acl$(e)

STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe

!include $(%ROOT)/mk/appsos2.mk
