#
# OS/3 (osFree) boot sequence project
# common make macros.
# (c) osFree project
# valerius, 2006/10/30
#

!ifndef __bootseq_mk__
!define __bootseq_mk__

!include $(%ROOT)/tools/mk/all.mk

ADD_COPT   = -i=$(ROOT)$(SEP)include -i=$(ROOT)$(SEP)include$(SEP)uFSD  -i=. -i=.. $(ADD_COPT) -bt=OS2
ADD_ASMOPT = -i=. -i=.. $(ADD_ASMOPT) -bt=OS2

RIP          = $(REXX) ripzeroes.cmd
GENREL       = $(REXX) genrel.cmd

#
# ...
#

!endif
