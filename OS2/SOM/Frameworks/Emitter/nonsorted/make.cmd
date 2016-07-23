@echo off

for %%a in (scentry scmethod scclass sctdef scbase scenum scstruct scunion scenumnm scconst scdata scattrib sccommon scmeta scmodule scpass scemit somstrt sctmplt def) do sc -p -s"h;ih;xh;xih" %%a.idl

cl /c /DWIN32_LEAN_AND_MEAN /D_WIN32 /DHAVE_CONFIG_HPP /MD /I. rhbseh2.c
lib rhbseh2.obj -OUT:rhbseh2.lib
cl /D_USE_SOME_ /DWIN32_LEAN_AND_MEAN /D_WIN32 /DHAVE_CONFIG_HPP /MD /I. rhbsc.cpp rhbidl.cpp rhbscapi.cpp rhbscemt.cpp rhbschdr.cpp rhbscsome.cpp rhbscir.cpp rhbsckih.cpp rhbscpp.cpp rhbsctc.cpp rhbsctyp.cpp ..\lib\somtk.lib some.lib

cl /D_USE_SOME_ /DWIN32_LEAN_AND_MEAN /D_WIN32 /DHAVE_CONFIG_HPP /MD emitlib.cpp scentry.cpp scmeta.cpp sctdef.cpp scstruct.cpp scenum.cpp scunion.cpp scenumnm.cpp scclass.cpp scconst.cpp scbase.cpp scattrib.cpp sccommon.cpp scpass.cpp scdata.cpp scmodule.cpp scemit.cpp somstrt.cpp sctmplt.cpp scmethod.cpp def.cpp /LD /Fesome.dll some.def ..\lib\somtk.lib

rhbsc /esome test.idl 

goto bexit

cl test.c ..\lib\somtk.lib some.lib
:cl emitdef.c ..\lib\somtk.lib some.lib

:bexit
