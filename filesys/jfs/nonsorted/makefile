# $Id: makefile,v 1.1 2000/04/21 10:57:51 ktk Exp $
#
#  JFS top level makefile.
#

allall: all

all dep clean:
    @cd lib
    @echo Entering directory: .\lib
    $(MAKE) /nologo $@
    @cd ..\src
    @echo Entering directory: .\src
    $(MAKE) /nologo $@
    @cd ..
    @echo Entering directory: .\

