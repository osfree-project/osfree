@echo off
IF NOT DEFINED SOMBASE CALL env.cmd

for %%a in (scentry scmethod scclass sctdef scbase scenum scstruct scunion scenumnm scconst scdata scattrib sccommon scmeta scmodule scpass scemit somstrt sctmplt scstring scseqnce scparm scusrtyp) do sc -p -s"h;ih;xh;xih" %%a.idl

: some.dll
cl /D_USE_SOME_ /DWIN32_LEAN_AND_MEAN /D_WIN32 /DHAVE_CONFIG_HPP /MD /I. emitlib.cpp scentry.cpp scmeta.cpp sctdef.cpp scstruct.cpp scenum.cpp scunion.cpp scenumnm.cpp scclass.cpp scconst.cpp scbase.cpp scattrib.cpp sccommon.cpp scpass.cpp scdata.cpp scmodule.cpp scemit.cpp somstrt.cpp sctmplt.cpp scmethod.cpp scstring.cpp scseqnce.cpp scparm.cpp scusrtyp.cpp /LD /Fesome.dll some.def somtk.lib

: emith.dll
for %%a in (hemit) do sc -p -s"h;ih;xh;xih" %%a.idl
cl /D_USE_SOME_ /DWIN32_LEAN_AND_MEAN /D_WIN32 /DHAVE_CONFIG_HPP /MD /I. hemit.cpp /LD /Feemith.dll emith.def some.lib somtk.lib

: emitdef.dll
for %%a in (defemit) do sc -p -s"h;ih;xh;xih" %%a.idl
cl /D_USE_SOME_ /DWIN32_LEAN_AND_MEAN /D_WIN32 /DHAVE_CONFIG_HPP /MD /I. defemit.cpp emitdef.cpp /LD /Feemitdef.dll emitdef.def some.lib somtk.lib

: emitlnk.dll
for %%a in (lnkemit) do sc -p -s"h;ih;xh;xih" %%a.idl
cl /D_USE_SOME_ /DWIN32_LEAN_AND_MEAN /D_WIN32 /DHAVE_CONFIG_HPP /MD /I. lnkemit.cpp emitlnk.cpp /LD /Feemitlnk.dll emitlnk.def some.lib somtk.lib

cl /c /DWIN32_LEAN_AND_MEAN /D_WIN32 /DHAVE_CONFIG_HPP /MD /I. rhbseh2.c
lib rhbseh2.obj -OUT:rhbseh2.lib
cl /D_USE_SOME_ /DWIN32_LEAN_AND_MEAN /D_WIN32 /DHAVE_CONFIG_HPP /MD /I. rhbsc.cpp rhbidl.cpp rhbscemt.cpp rhbscsome.cpp rhbscpp.cpp rhbsctyp.cpp rhbsctxt.cpp somtk.lib


rhbsc /eh test.idl > log 2>&1

goto bexit

cl test.c somtk.lib some.lib
:cl emitdef.c somtk.lib some.lib

:bexit
