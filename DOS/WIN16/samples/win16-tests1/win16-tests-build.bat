set opt=-D_X86_ -DSTANDALONE -zW -i=C:\WATCOM\h\win;C:\WATCOM\h -zq -oaxt -d2 -w4 
wcc %opt% dosmem.c
wlink file dosmem.obj 

wcc %opt% file.c
wlink file file.obj

wcc %opt% dos_func.c
wcc %opt% dos_file.c
wlink @dos_file.lnk

wcc %opt% sysdir.c
wlink file sysdir.obj

wcc %opt% alloc.c
wlink file alloc.obj

wcc %opt% atom.c
wlink file atom.obj

wcc %opt% bitmap.c
wlink file bitmap.obj

wcc %opt% comm16.c
wlink file comm16.obj

wcc %opt% str.c
wlink file str.obj

wcc %opt% sys.c
wlink file sys.obj

wcc %opt% timer.c
wlink file timer.obj

wcc %opt% window.c
wlink file window.obj
