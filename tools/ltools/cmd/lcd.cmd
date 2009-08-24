@REM set Linux base directory for LDIR, LREAD AND LWRITE to command line parameter %1
@REM please note: Linux base directory must begin with '/'
@if ""%1=="" goto show
@set LDIR=%1
:show
@echo Linux base directory is   %LDIR%
