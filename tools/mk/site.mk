#
# Site-dependent
# variables.
#

!ifndef __site_mk__
!define __site_mk__

# Project root
!ifdef %ROOT
ROOT       = $(%ROOT)
!endif
# OpenWatcom directory
!ifdef %WATCOM
WATCOM     = $(%WATCOM)
!endif

!include $(%ROOT)/tools/mk/build.conf

# Version macro. Not to be changed for a long time yet.

_VENDOR=osFree
_VERSION=14.100
FILEVER=@$#$(_VENDOR):$(_VERSION)$#@

# Shell
!ifeq OS_SHELL 4OS/2
SHELL   = 4os2
!else ifeq OS_SHELL CMD
SHELL   = cmd
!else ifeq OS_SHELL Bourne
SHELL   = /bin/sh
!else ifeq OS_SHELL Cshell
SHELL   = /bin/csh
!else
SHELL   = $OS_SHELL
!endif

# Determine a type of OS:
!ifeq ENV OS2
UNIX = FALSE
!else ifeq ENV Windows
UNIX = FALSE
!else ifeq ENV WIN32
UNIX = FALSE
!else ifeq ENV WIN64
UNIX = FALSE
!else ifeq ENV LINUX
UNIX = TRUE
!else ifeq ENV FreeBSD
UNIX = TRUE
!else
! error Unsupported OS!
!endif

# Path separator
!ifeq UNIX FALSE
SEP        = \
!else
SEP        = /
!endif

# REXX interpreter
# (for OS/2 Classic REXX the interreter must be empty,
# as it is a library, not executable file)
!ifeq OS_REXX Classic
REXX    =
%REXX_DLL     = rexc
%REXXAPI_DLL  = rexcapi
!else ifeq OS_REXX Object
REXX    =
%REXX_DLL     = rexo
%REXXAPI_DLL  = rexoapi
!else ifeq OS_REXX Regina
!ifeq ENV OS2
REXX    =
%REXX_DLL     = regina
%REXXAPI_DLL  = regina
!else
REXX    = rexx
!endif
!else
REXX    = $OS_REXX
!endif

# PLATFORM =

# Files from which HDD image is built
FILESDIR   = $(ROOT)build$(SEP)bin

# Libraries dir (this is NOT standalone system libraries. This is path where
# to store libs generated during build process)
LIBDIR     = $(FILESDIR)$(SEP)build$(SEP)bin$(SEP)lib

# libc
LIBC       = clibs.lib

!endif
