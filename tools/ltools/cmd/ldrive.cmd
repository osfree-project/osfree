@REM set Linux default drive for LDIR, LREAD AND LWRITE to command line parameter %1
@if ""%1=="" goto show
@set LDRIVE=%1
:show
@echo Linux default drive is   %LDRIVE%
