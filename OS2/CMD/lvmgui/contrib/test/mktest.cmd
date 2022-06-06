icc /Ss /C /Ti+ /Tm+ /I.. ..\lvm_ctls.c
icc /Ss /C /Ti+ /Tm+ /I.. testctls.c
ilink testctls.obj lvm_ctls.obj /PMTYPE:PM /DEBUG /ST:65536
cd ..
rc test/testctls.rc test/testctls.exe

