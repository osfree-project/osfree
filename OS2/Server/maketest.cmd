wcc386 -db -d2 -bt=os2v2 -i..\..\include;c:\watcom\h;c:\watcom\h\os2 -s -g=DGROUP mini33.c
wlink debug watcom sys os2v2 name mini33.exe f mini33 imp DosPutMessage MSG.5 op start=startup,st=32k,nod,map option internalrelocs
