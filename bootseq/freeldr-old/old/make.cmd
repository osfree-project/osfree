del *.obj
del os2ldr
rem wasm -ms cstart_t.ASM
wasm -ms cmodel.ASM
wasm -ms i4m.ASM
wasm -3p -ms FREELDRA.ASM
wasm -3p -ms PERIPHS.ASM
REM wcc -3 -d0 -s -od -ms boot.c
rem wcc -3 -d0 -s -od -ms builtins.c
rem wcc -3 -d0 -s -od -ms char_io.c
rem wcc -3 -d0 -s -od -ms strlen.c
rem wcc -3 -d0 -s -od -ms strupr.c
wcc -3 -d0 -s -od -ms segread.c
rem wcc -3 -d0 -s -od -ms prtf.c
rem wcc -3 -d0 -s -od -ms itoa.c
rem wcc -3 -d0 -s -od -ms ltoa.c
rem wcc -3 -d0 -s -od -ms alphabet.c
rem wcc -3 -d0 -s -od -ms vsprintf.c

wcc -3 -d0 -s -od -ms strcpy.c
rem wcc -3 -d0 -s -od -ms sprintf.c
wcc -3 -d0 -s -od -ms freeldrc.c
rem file i4m.obj  file alphabet.obj
rem file strupr.obj file itoa.obj file ltoa.obj file strlen.obj
rem file prtf.obj file vsprintf.obj
rem dile sprintf.obj file boot.obj
rem file char_io.obj file builtins.obj
wlink file FREELDRA.obj file cmodel.obj file segread.obj file strcpy.obj file FREELDRC.obj file PERIPHS.obj file i4m.obj name os2ldr format dos com option nodef option map=os2ldr.map
ren os2ldr.com  os2ldr
