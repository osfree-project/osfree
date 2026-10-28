#
# A Makefile for pmclient.dll
# (c) osFree project
#

PROJ = pmclient
TRGT = $(PROJ).dll
DESC = FreePM client
srcfiles     = $(p)Fc_config$(e) $(p)init$(e)
ADD_COPT     = -i=. -sg -of+ -bd -3s
ADD_LINKOPT  = segment   type DATA nonshared
NOLIBS       = 1
DLL          = 1
DLLOPT       = initinstance terminstance
OPTIONS      = manyautodata, caseexact

EXPORTS      = &
    PmcPostCommand             .1, &
    PmcSendData                .2, &
    PmcGetData                 .3, &
    PmcSendCommand             .4, &
    PmcSendCommandData         .5

DEST         = os2$(SEP)dll
#UNI2H        = 1

!include $(%ROOT)tools/mk/appsos2_pm.mk



