#
# A Makefile for bdcall32.dll
# (c) osFree project,
# author, date
#

PROJ     = bdcall32
TRGT     = $(PROJ).dll
DESC     = osFree BiDirectional Language APIs
#OBJS    = $(PATH)..$(SEP)shared$(SEP)stub.$(O)
srcfiles = $(p)..$(SEP)bdcalls$(SEP)bdcalls$(e) # $(p)bdcall32$(e)
ADD_COPT = -bd -i=$(%WATCOM)$(SEP)h$(SEP)os2 # until UniAPI modules will be ready
ADD_LINKOPT = lib os2386.lib lib clib3r.lib segment type DATA shared
DLL      = 1
DLLOPT   = initinstance terminstance
OPTIONS  = manyautodata

EXPORTS  = &
 NLS_16CONVERTBIDINUMERICS.1=NLS16CONVERTBIDINUMERICS, &
 NLS_16CONVERTBIDISTRING.2=NLS16CONVERTBIDISTRING, &
 NLS_16INVERSESTRING.7=NLS16INVERSESTRING, &
 NLS_16EDITSHAPE.9=NLS16EDITSHAPE, &
 NLS_16SHAPEBIDISTRING.10=NLS16SHAPEBIDISTRING, &
 NlsConvertBidiNumerics.14=NlsConvertBidiNumerics, &
 NlsConvertBidiString.15=NlsConvertBidiString, &
 NlsInverseString.20=NlsInverseString, &
 NlsEditShape.22=NlsEditShape, &
 NlsShapeBidiString.23=NlsShapeBidiString
# RC_16BIDIATTR.50=RC_16BIDIATTR, &
# Set_Round_Trip.100=Set_Round_Trip, &
# layout_32_object_create.206=layout_32_object_create, &
# layout_32_object_destroy.207=layout_32_object_destroy, &
# layout_32_object_transform.208=layout_32_object_transform, &
# layout_32_object_editshape.209=layout_32_object_editshape, &
# layout_32_object_setvalues.210=layout_32_object_setvalues, &
# layout_32_object_getvalues.211=layout_32_object_getvalues

DEST = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2_cmd.mk
