SOM COMPILER WATCOM LINKER EMITTER README FILE
==============================================

Installation

Put DLL to SOM\DLL. Put EFW file to SOM\INCLUDE dir.

Usage

sc -slnk file.idl

Supported global modifiers:

 "dllname=name.dll"          add 'name name.dll' string to link file.
 "file=file1.obj,file2.obj"  add 'file file1.obj,file2.obj' string to lnk file.
 "lib=file1.lib,file2.lib"   add 'lib file1.lib,file2.lib' string to lnk file.
 "include=file.lnk"          include content of file.lnk to output file.

Controlling of library/dll name:

'name' option of linker file controlled in following way:
a) if exists dllname global modifier (via -m option) then it is used else
b) if exists local dllname modifier then it is used else
c) if exists filestem then it is used

