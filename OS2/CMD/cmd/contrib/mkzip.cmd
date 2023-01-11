@echo off
copy release\4os2.exe distrib
copy release\shralias.exe distrib
copy release\option2.exe distrib
copy release\keystack.exe distrib
copy release\jpos2dll.dll distrib
copy release\4os2h.msg distrib
copy release\4os2.inf distrib
copy 4os2.ico distrib
copy license.txt distrib
copy readme.1st distrib
zip -j 4os2 distrib\*
