@if "%OS%"=="Windows_NT" goto NT

:dos
@ldirDOS %LOPTIONS% %1 %2 %3 %4 %5 %6 %7 %8 %9
@goto ende

:NT
@copy ldirNT.exe ldir.exe
@ldirNT %LOPTIONS% %1 %2 %3 %4 %5 %6 %7 %8 %9

:ende
