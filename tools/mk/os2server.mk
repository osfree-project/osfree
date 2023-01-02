#
# OS/2 personality servers
# include file
#

!ifndef __server_mk__
!define __server_mk__

#
# ARCH variable defines
# host operating system,
# currently: OS2, Win32, Linux
#
# You must define such variable like this:
# set env=(os2|win32|linux)
# ARCH=$(%env)
#

32_BITS = 1

!ifdef ALIASES
comma = ,
!else
comma =
!endif

!include $(%ROOT)/tools/mk/dirs.mk

SrvDir = $(%ROOT)OS2$(SEP)Server$(SEP)

ADD_COPT = $(ADD_COPT) -d2 -db -i=$(SrvDir)Shared$(SEP)modmgr -i=. -i=.. &
           -i=$(SrvDir)include &
#           -std=c99 -O1 -g2 -Wall

ADD_LINKOPT = $(ADD_LINKOPT) debug all # lib os2server_shared.lib, os2server_$(ARCH).lib

!ifeq ARCH os2

ADD_COPT    = -u__LINUX__ -u__WIN32__ -u__WINNT__ -d__OS2__ &
              $(ADD_COPT) -i=$(SrvDir)os2 &
              -i=$(%WATCOM)$(SEP)h -i=$(%WATCOM)$(SEP)h$(SEP)os2
ADD_LINKOPT = libpath $(%WATCOM)$(SEP)lib386$(SEP)os2 &
              $(ADD_LINKOPT) lib libmmap.lib
ALIASES = $(ALIASES) $(comma)mmap_=_mmap,munmap_=_munmap
OPTIONS = internalrelocs
!include $(%ROOT)/tools/mk/appsos2.mk

!else ifeq ARCH linux

ADD_COPT    = -u__OS2__ -u__WIN32__ -u__WINNT__ -d__LINUX__ &
              $(ADD_COPT) -i=$(SrvDir)linux &
              -i=$(%WATCOM)$(SEP)lh
ADD_LINKOPT = libpath $(%WATCOM)$(SEP)lib386$(SEP)linux &
              $(ADD_LINKOPT)
#ALIASES = $(ALIASES) $(comma)_PrcExecuteModule=PrcExecuteModule_
!include $(%ROOT)/mk/appslnx.mk

!else ifeq ARCH win32

ADD_COPT    = -u__OS2__ -u__LINUX__ -d__WIN32__ -d__WINNT__ &
              $(ADD_COPT) -i=$(SrvDir)win32 &
              -i=$(%WATCOM)$(SEP)h -i=$(%WATCOM)$(SEP)h$(SEP)nt &
              -i=$(%WATCOM)$(SEP)h$(SEP)os2

ADD_LINKOPT = lib kernel32 libpath $(%WATCOM)$(SEP)lib386$(SEP)nt &
              $(ADD_LINKOPT)
OPTIONS = OFFSET=524288
# -mthreads
!include $(%ROOT)/tools/mk/appsw32.mk

!endif

!endif
