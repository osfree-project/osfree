icc.exe  /Tx /Gh /Ti+ /Gm+ /Tm /Fb /C lvmtest.cpp
ilink /NOFREE /BR /NOE /DE /PMTYPE:VIO lvmtest.obj,lvmtest.exe,NUL.MAP,LVM.LIB,NUL.DEF

