@echo off
set dir4os2=c:\bin\4os2
unlock %dir4os2%\jpos2dll.dll
copy release\jpos2dll.dll %dir4os2%
unlock %dir4os2%\4os2.exe
unlock %dir4os2%\shralias.exe
copy release\4os2.exe %dir4os2%
copy release\shralias.exe %dir4os2%
copy release\keystack.exe %dir4os2%
copy release\option2.exe %dir4os2%
copy release\4os2.inf %dir4os2%
set dir4os2=
