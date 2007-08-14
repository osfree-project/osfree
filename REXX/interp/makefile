# $Id: makefile,v 1.5 2004/08/21 14:48:36 prokushev Exp $

ROOT=..\..\..

EXEDIR = $(ROOT)\bin\os2
DLLDIR = $(EXEDIR)\dll

!include $(ROOT)\makefile.inc

all
  -@del /q $(EXEDIR)\rexx.exe
  -@del /q $(EXEDIR)\regina.exe
  -@del /q $(EXEDIR)\execiser.exe
  -@del /q $(DLLDIR)\regina.dll
  @%create wrapper.cmd
  @%append wrapper.cmd @set REGINA_SRCDIRN=.
  @%append wrapper.cmd @$(MAKE) -f makefile.os2.wcc
  @wrapper.cmd
  @del /q wrapper.cmd
  @move *.exe $(EXEDIR)
  @move regina.dll $(DLLDIR)

# Clean target
clean : .symbolic
    @if exist *.obj @del /q *.obj
    @if exist *.dlo @del /q *.dlo
    @if exist *.dll @del /q *.dll
    @if exist *.exe @del /q *.exe
    @if exist *.res @del /q *.res
    @if exist *.msg @del /q *.msg
    @if exist *.map @del /q *.map
    @if exist *.wmp @del /q *.wmp
    @if exist *.lnk @del /q *.lnk
    @if exist *.err @del /q *.err
    @if exist *.lib @del /q *.lib
