#
# Site-dependent
# variables.
#

# Project root
ROOT       = $(%ROOT)
# OpenWatcom directory
WATCOM     = $(%WATCOM)

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

# REXX interpreter
# (for OS/2 Classic REXX the interreter must be empty,
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

# Determine a type of OS:
!     ifeq ENV OS/2
UNIX = FALSE
!else ifeq ENV Windows
UNIX = FALSE
!else ifeq ENV Linux
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

# Files from which HDD image is built
FILESDIR   = $(ROOT)$(SEP)files

# Libraries dir (this is NOT standalone system libraries. This is path where
# to store libs generated during build process)
LIBDIR     = $(ROOT)$(SEP)lib

# libc
LIBC       = clibs.lib
