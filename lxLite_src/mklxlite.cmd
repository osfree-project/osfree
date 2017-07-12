@echo off
rem 
rem win32 VPC build
rem requires VPC, Borland`s brc32 and upx in path
rem 
@brc32 -r lxlite.rc lxlite.res
@brc32 -r os2api.rc os2api.res
@vpc -b -Ucommon lxLite.pas
@if exist lxLite.exe upx -9 lxLite.exe
