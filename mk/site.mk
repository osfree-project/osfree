#
# Site-dependent
# variables.
#

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

# Libraries dir
!ifeq UNIX FALSE
LIBDIR     = $(%WATCOM)\lib286\dos;$(ROOT)\lib;$(%WATCOM)\lib286\os2
!else
LIBDIR     = $(%WATCOM)/lib286/dos:$(ROOT)/lib:$(%WATCOM)/lib286/os2
!endif

# libc
LIBC       = clibs.lib
