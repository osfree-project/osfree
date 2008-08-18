#
# A Makefile for pmdrag.dll
# (c) osFree project,
# author, date
#

PROJ     = pmmle
DESC     = Multi-Line Editor Control
srcfiles = $(p)..$(SEP)shared$(SEP)stub$(e)
ADD_COPT = -i=..\include
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib segment type DATA shared
DLL     = 1
DLLOPT  = initinstance terminstance
OPTIONS = manyautodata
IMPORTS = MLE_INITIALIZE                   PMCTLS.31, &
          MULTILINEEDITPROC                PMCTLS.32, &
          BDMULTILINEEDITPROC              PMCTLS.33
EXPORTS = MLE_INITIALIZE.                  00001,  &
          MULTILINEEDITPROC.               00002,  &
          BDMULTILINEEDITPROC.             00003
DEST    = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk
