@echo off
rem VP path:
set VPCPATH=f:\dev\vp21
rem Platform:
set p=os2
set path=%path%;%vpcpath%\bin.%p%
set beginlibpath=%vpcpath%\bin.%p%
set binpath=%vpcpath%\bin.%p%
%binpath%\vpc.exe -B -R%vpcpath%\res.%p% -E%vpcpath%\out.%p% -O%vpcpath%\out.%p%\units -L%vpcpath%\lib.%p% -L%vpcpath%\units.%p% -I. -I%vpcpath%\source\rtl -I%vpcpath%\source\tv -U. -U%vpcpath%\units.%p% -U%vpcpath%\source\rtl -U%vpcpath%\source\%p% -U%vpcpath%\source\tv freeinst.pas
