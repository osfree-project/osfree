DBG = 0

!ifeq DBG 1
OPT         = -d3 -of -od
!else
OPT         = -d0 -oteanx
!endif

PROJ        = rxuls
TRGT        = $(PROJ).dll
DESC        = REXX Universal Language Support functions
ADD_COPT    = -i=$(MYDIR) -bm -wx -s -mf $(OPT) -6s -fp6 # -6r
ADD_LINKOPT = lib libuls.lib, libconv.lib
# UNI2H       = 1
DLL         = 1
DLLOPT      = initinstance terminstance
OPTIONS     = manyautodata
STACKSIZE   = 0x20000
C = r

srcfiles    = $(p)rxuls$(e)
EXPORTS     = &
            ULSLoadFuncs.1               = ULSLoadFuncs, &
            ULSDropFuncs.2		 = ULSDropFuncs, &
            ULSVersion.3		 = ULSVersion,   &
            ULSConvertCodepage.4	 = ULSConvertCodepage, &
            ULSCountryLocale.5           = ULSCountryLocale,   &
            ULSFindAttr.6		 = ULSFindAttr,  &
            ULSGetLocales.7		 = ULSGetLocales, &
            ULSQueryAttr.8		 = ULSQueryAttr, &
            ULSQueryLocaleItem.9	 = ULSQueryLocaleItem, &
            ULSTransform.10		 = ULSTransform, &
            ULSPutUnicodeClipboard.11	 = ULSPutUnicodeClipboard, &
            ULSGetUnicodeClipboard.12	 = ULSGetUnicodeClipboard

DEST        = os2$(SEP)dll

!include $(%ROOT)tools/mk/appsos2_cmd.mk
