#
#
#
#

!ifndef __serverlib_mk__
!define __serverlib_mk__

#
# ARCH variable defines
# host operating system,
# currently: OS2, Win32, Linux
#
# You must define such variable like this:
# wmake all ARCH=OS2
#

32_BITS = 1
CLEAN_ADD = *.inf *.cmd *.msg *.pl *.ru *.rsf *.c *.h
SrvDir = $(%ROOT)OS2$(SEP)Server$(SEP)

ADD_COPT = $(ADD_COPT) -d2 -db &
           -i=$(SrvDir)Shared$(SEP)modmgr &
           -i=$(SrvDir)include -i=. -i=..

#           -std=c99   $(COPT_TARGET) -O1 -g2 -Wall

ADD_LINKOPT = debug all lib os2server_shared_$(ARCH).lib, os2server_$(ARCH).lib

!ifeq ARCH os2

PLATFORM = os2
ADD_COPT =            -u__WINNT__ -u__WIN32__ -u__LINUX__ -d__OS2__ &
                      $(ADD_COPT) -i=$(SrvDir)os2 &
                      -i=$(%WATCOM)$(SEP)h &
                      -i=$(%ADD_INC) &
#                      -i=$(%ROOT)$(SEP)build$(SEP)include &
#                      -i=$(%ROOT)$(SEP)build$(SEP)include$(SEP)os2 &
                      -i=$(%ROOT)$(SEP)include &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3 &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)pm &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)GDlib &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)zlib &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)lpng &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)jpeglib &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)libtiff &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)gbm &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)pdcurses &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)glib &
                      -i=$(%WATCOM)$(SEP)h$(SEP)os2 &
                      -bt=os2
ADD_LINKOPT = libpath $(%WATCOM)$(SEP)lib386$(SEP)os2 &
              $(ADD_LINKOPT) lib libmmap.lib

!else ifeq ARCH linux

PLATFORM = linux
EXE_SUFFIX=l
ADD_COPT =            -u__OS2__ -u__WIN32__ -u__WINNT__ -d__LINUX__ &
                      $(ADD_COPT) -i=$(SrvDir)Linux &
                      -i=$(%WATCOM)$(SEP)lh &
                      -i=$(%ROOT)$(SEP)build$(SEP)include &
                      -i=$(%ROOT)$(SEP)build$(SEP)include$(SEP)os2 &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3 &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)pm &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)GDlib &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)zlib &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)lpng &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)jpeglib &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)libtiff &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)gbm &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)pdcurses &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)glib &
                      -bt=linux
ADD_LINKOPT = libpath $(%WATCOM)$(SEP)lib386$(SEP)linux &
              $(ADD_LINKOPT)

!else ifeq ARCH win32

PLATFORM = nt
ADD_COPT =  -u__OS2__ -u__LINUX__ -d__WIN32__ -d__WINNT__ &
                      $(ADD_COPT) -i=$(SrvDir)win32 &
                      -i=$(%WATCOM)$(SEP)h$(SEP)os2 &
                      -i=$(%WATCOM)$(SEP)h$(SEP)nt &
                      -i=$(%ROOT)$(SEP)build$(SEP)include &
                      -i=$(%ROOT)$(SEP)build$(SEP)include$(SEP)os2 &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3 &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)pm &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)GDlib &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)zlib &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)lpng &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)jpeglib &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)libtiff &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)gbm &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)pdcurses &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)glib &
                      -bt=nt

ADD_LINKOPT = libpath $(%WATCOM)$(SEP)lib386$(SEP)nt &
              $(ADD_LINKOPT)

!endif

!include $(%ROOT)/tools/mk/libs.mk

!endif
