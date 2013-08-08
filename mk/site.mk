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

!include $(ROOT)/build.conf

# Version macro. Not to be changed for a long time yet.

_VENDOR=osFree
_VERSION=14.100
FILEVER=@$#$(_VENDOR):$(_VERSION)$#@

# Shell
!     ifeq OS_SHELL 4OS/2
SHELL   = 4os2
!else ifeq OS_SHELL CMD
SHELL   = cmd
!else ifeq OS_SHELL Bourne
SHELL   = /bin/sh
!else ifeq OS_SHELL Cshell
SHELL   = /bin/csh
!else
SHELL   = OS_SHELL
!endif

# Determine a type of OS: (case sensitive comparision)
!ifeq ENV OS2
UNIX = FALSE
!else ifeq ENV Windows
UNIX = FALSE
!else ifeq ENV WIN32
UNIX = FALSE
!else ifeq ENV LINUX
UNIX = TRUE
!else ifeq ENV FreeBSD
UNIX = TRUE
!else
! error "Unsupported OS! Is ENV or OS not set? $(ENV) $(%OS)"
!endif


# Path separator
!ifeq UNIX FALSE
SEP        = \
!else
SEP        = /
!endif

# REXX interpreter
# (for OS/2 Classic REXX the interpreter must be empty,
# as it is a library, not executable file)
!     ifeq OS_REXX Classic
REXX    =
!else ifeq OS_REXX Object
REXX    =
!else ifeq OS_REXX Regina
REXX    = rexx
!else
REXX    = $OS_REXX
!endif


# Files from which HDD image is built. Use the ROOT var from env.
FILESDIR   = $(%ROOT)$(SEP)bin

# Libraries dir (this is NOT standalone system libraries. This is path where
# to store libs generated during build process)
LIBDIR     = $(FILESDIR)$(SEP)bin$(SEP)lib

# libc
LIBC       = clibs.lib

!endif
