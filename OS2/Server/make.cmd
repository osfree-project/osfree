set path=f:\dev\watcom\binp
set watcom=f:\dev\watcom
wcl386 shared\os2server\main.c shared\token.c shared\modmgr\modmgr.c shared\ixfmgr\ixfmgr.c shared\ixfmgr\lx\lx.c shared\ixfmgr\ne\ne.c shared\ixfmgr\lx\fixuplx.c shared\ixfmgr\lx\loadobjlx.c  shared\ixfmgr\lx\modlx.c shared\ixfmgr\lx\execlx.c shared\ixfmgr\lx\debuglx.c shared\memmgr\memmgr.c os2\io.c os2\native_dynlink.c shared\cfgparser\cfgparser.c shared\processmgr\processlx.c -iinclude;c:\watcom\h;c:\watcom\h\os2 -"op map option internalrelocs" -d2
