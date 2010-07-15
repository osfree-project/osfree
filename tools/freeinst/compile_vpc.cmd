set VPCPATH=f:\dev\vp21
set binpath=%vpcpath%\bin.os2
%binpath%\vpc.exe -B -R%vpcpath%\res.os2 -E%vpcpath%\out.os2 -O%vpcpath%\out.os2\units -L%vpcpath%\lib.os2 -L%vpcpath%\units.os2 -I. -I%vpcpath%\source\rtl -I%vpcpath%\source\tv -U. -U%vpcpath%\units.os2 -U%vpcpath%\source\rtl -U%vpcpath%\source\os2 -U%vpcpath%\source\tv freeinst.pas
