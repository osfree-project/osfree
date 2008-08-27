set path=c:\watcom\binnt
set watcom=c:\watcom
wcl386 shared\os2server\main.c shared\modmgr\modmgr.c shared\ixfmgr\ixfmgr.c shared\ixfmgr\lx\lx.c shared\ixfmgr\ne\ne.c shared\ixfmgr\lx\fixuplx.c shared\ixfmgr\lx\loadobjlx.c  shared\ixfmgr\lx\modlx.c shared\ixfmgr\lx\execlx.c shared\ixfmgr\lx\debuglx.c shared\memmgr\memmgr.c windows\io.c windows\mmap.c windows\native_dynlink.c shared\cfgparser\cfgparser.c shared\processmgr\processlx.c -iinclude;c:\watcom\h;c:\watcom\h\os2;c:\watcom\h\nt -"op map" -d2
