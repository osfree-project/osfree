#
# A Makefile for MiniCMD
# (c) osFree project,
# prokushev, 2008
#

PROJ = minicmd33
DESC = Minimalistic Command line processor
ADD_COPT  = -db -d2 -s -g=DGROUP -i=$(MYDIR)..$(SEP)..$(SEP)include
OPTIONS   = start=startup,internalrelocs,nod
DEBUG = watcom all
STACKSIZE = 32k
#defines object file names in format objname.$(O)
srcfiles = $(p)minicmd$(e) $(p)commands$(e) $(p)strnlen$(e) $(p)strncmp$(e)
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe

IMPORTS = KbdStringIn           EMXWRAP.209, &
          DosError              DOSCALLS.212, &
          DosQueryCurrentDir    DOSCALLS.274  , &
          DosQueryCurrentDisk   DOSCALLS.275  , &
          DosSetCurrentDir      DOSCALLS.255  , &
          DosExecPgm            DOSCALLS.283  , &
          DosSetDefaultDisk     DOSCALLS.220

!include $(%ROOT)/mk/appsos2.mk






