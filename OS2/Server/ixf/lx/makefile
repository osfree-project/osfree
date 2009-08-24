#
#
#
#

ARCH        =  $(%SERVERENV)

PROJ        =  lx
DESC        =  LX executable format support
srcfiles    =  $(p)lx$(e) $(p)debuglx$(e) $(p)fixuplx$(e) &
               $(p)modlx$(e) $(p)loadobjlx$(e)
ADD_COPT    =
ADD_LINKOPT =  lib clib3r,libmmap.lib

DLL         =  1
OPTIONS     =  manyautodata, caseexact
#DLLOPT     =  initglobal

#IMPORTS     =

EXPORTS     =  LXIdentify=ixfIdentify.1, &
               LXIdentify=ixfLoad.2, &
               LXIdentify=ixfFixup.3

ALIASES     =  ixfIdentify=LXIdentify_, ixfLoad=LXLoad_, ixfFixup=LXFixup_

TARGETS     =  $(PATH)$(PROJ).dll
DEST        =  .

!include $(%ROOT)/mk/os2server.mk
