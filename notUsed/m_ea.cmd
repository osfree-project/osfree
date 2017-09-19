@echo off
echo This command file will detach EAs of lxlite\*.exe files into
echo separate .EA files! Do you still wish to continue?
echo press 'Any' key
pause >nul
eautil lxLite\lxLite.exe lxLite.EA /s /p /r
eautil lxLite\chCase.exe chCase.EA /s /p /r
eautil lxLite\noEA.exe noEA.EA /s /p /r
eautil lxLite\unLock.exe unLock.EA /s /p /r
eautil lxLite\install.cmd install.EA /s /p /r
