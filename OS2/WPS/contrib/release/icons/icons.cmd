@echo off
copy ..\bin\icons.dll
rc -x2 icons.rc icons.dll
del icons.res

echo The ICONS.DLL file has been recreated.
echo You will need to restart the WPS to have XFolder
echo copy this file to its /BIN subdirectory. You cannot
echo do this yourself because /BIN/ICONS.DLL is currently
echo locked.
pause
