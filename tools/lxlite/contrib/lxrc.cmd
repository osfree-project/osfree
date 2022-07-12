@echo off
 rc -n lxlite.rc ..\lxlite.exe
 if errorlevel 1 goto end
 rc -n os2api.rc ..\lxlite.exe
 if errorlevel 1 goto end
 ..\lxlite.exe /yur ..\lxlite.exe
 del *.res 1>nul 2>nul
:end
