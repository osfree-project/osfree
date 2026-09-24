!ifndef __build_dispatcher_mk__
!define __build_dispatcher_mk__

# ============================================================
# TARGET_API / TARGET_BITS matrix (top-level dispatch)
# ============================================================
# TARGET_API     16-bit                 32-bit                 64-bit
# -------------  ---------------------  ---------------------  ---------------------
# BARE           ERROR (reserved)       ERROR (reserved)       ERROR (reserved)
# BIOS           ERROR (reserved)       ERROR (reserved)       ERROR (reserved)
# ABIOS          ERROR (reserved)       ERROR (reserved)       ERROR (reserved)
# UEFI           ERROR (reserved)       ERROR (reserved)       ERROR (reserved)
# DOS            --> DOS_CLASS          ERROR (reserved)       ERROR (reserved)
# DPMI           ERROR (reserved)       --> ERROR (reserved)   ERROR (reserved)
# WIN            --> WIN16_CLASS        --> WIN32_CLASS        ERROR (reserved)
# OS2            --> OS2_16_CLASS       --> OS2_32_CLASS (def) ERROR (reserved)
# HOST           --> HOST_CLASS         --> HOST_CLASS         --> HOST_CLASS
#
# Note: For HOST, TARGET_BITS is not defaulted in this file.
#       "default" marks the TARGET_BITS used when not explicitly set.
#       Entries with "--> TABLE" defer to class/subclass sub-tables below.
#       DOS, DPMI and HOST do not validate TARGET_BITS explicitly.

# ============================================================
# Default TARGET_BITS per TARGET_API
# ============================================================
# TARGET_API     Default bits   Allowed bits
# -------------  -------------  -----------------
# DOS            16             16, 32
# DPMI           16             16, 32
# WIN            16             16, 32, 64(reserved)
# OS2            32             16, 32
# HOST           not defined
# BARE / BIOS    ERROR (reserved)

!ifndef TARGET_BITS
!  ifeq TARGET_API DOS
!    define TARGET_BITS 16
!  else ifeq TARGET_API DPMI
!    define TARGET_BITS 16
!  else ifeq TARGET_API WIN
!    define TARGET_BITS 16
!  else ifeq TARGET_API OS2
!    define TARGET_BITS 32
!  else ifeq TARGET_API HOST
!    ifdef TARGET_BITS
!      undef TARGET_BITS
!    endif
!  else ifeq TARGET_API BARE
!    error BARE target is reserved and not yet implemented
!  else ifeq TARGET_API BIOS
!    error BIOS target is reserved and not yet implemented
!  else
!    error Unknown TARGET_API: $(TARGET_API)
!  endif
!endif

# ============================================================
# Bitness validation and convenience flags
# ============================================================
# TARGET_BITS    32_BITS     64_BITS
# -------------  ----------  ----------
# 16             0           0
# 32             1           0
# 64             0           1
!ifdef TARGET_BITS
!  ifeq TARGET_BITS 16
!    define 16_BITS 1
!    define 32_BITS 0
!    define 64_BITS 0
!  else ifeq TARGET_BITS 32
!    define 16_BITS 0
!    define 32_BITS 1
!    define 64_BITS 0
!  else ifeq TARGET_BITS 64
!    define 16_BITS 0
!    define 32_BITS 0
!    define 64_BITS 1
!  else
!    error Unsupported TARGET_BITS: $(TARGET_BITS) (expected 16, 32 or 64)
!  endif
!endif

# ============================================================
# Default TARGET_CLASS and TARGET_SUBCLASS
# ============================================================
# TARGET_CLASS    Default TARGET_SUBCLASS
# -------------   ----------------------
# APPLICATION     CONSOLE (WIN: GUI)
# LIBRARY         DYNAMIC (DOS, HOST: STATIC)
# DRIVER          (required except DOS: no subtype needed)
!ifndef TARGET_CLASS
!  define TARGET_CLASS APPLICATION
!endif

!ifndef TARGET_SUBCLASS
!  ifeq TARGET_CLASS APPLICATION
!    ifeq TARGET_API WIN
!      define TARGET_SUBCLASS GUI
!    else
!      define TARGET_SUBCLASS CONSOLE
!    endif
!  else ifeq TARGET_CLASS LIBRARY
!    ifeq TARGET_API DOS
!      define TARGET_SUBCLASS STATIC
!    else ifeq TARGET_API HOST
!      define TARGET_SUBCLASS STATIC
!    else
!      define TARGET_SUBCLASS DYNAMIC
!    endif
!  else ifeq TARGET_CLASS DRIVER
!    ifneq TARGET_API DOS
!      error TARGET_SUBCLASS is required for DRIVER class on $(TARGET_API)
!    endif
!  else
!    error Unknown TARGET_CLASS: $(TARGET_CLASS). Supported: APPLICATION, LIBRARY, DRIVER.
!  endif
!endif

# -------------------------------------------------------------
# Define DLL for DLLs
# -------------------------------------------------------------

!ifeq TARGET_CLASS LIBRARY
!  ifeq TARGET_SUBCLASS DYNAMIC
!    define DLL 1
!  endif
!endif

# -------------------------------------------------------------
# Define TRGT with extension
# -------------------------------------------------------------
!ifdef TARGET_EXT
TRGT = $(PROJ).$(TARGET_EXT)
!else

!ifeq TARGET_CLASS APPLICATION

!ifdef DLL
TRGT = $(PROJ).dll
!else
TRGT = $(PROJ).exe
!ifeq COM 1
TRGT = $(PROJ).com
!endif
!endif

!else ifeq TARGET_CLASS LIBRARY

!ifeq TARGET_SUBCLASS STATIC
TRGT = $(PROJ).lib
!else
TRGT = $(PROJ).dll
!endif

!else ifeq TARGET_CLASS DRIVER

TRGT = $(PROJ).$(TARGET_EXT)

!else

!error Unknown TARGET_CLASS: $(TARGET_CLASS). Supported: APPLICATION, LIBRARY, DRIVER.

!endif

!endif

# -------------------------------------------------------------
# Platform-specific dispatch
# -------------------------------------------------------------

# --- DOS ---
!ifeq TARGET_API DOS
# ============================================================
# DOS classes (TARGET_API=DOS)
# ============================================================
# TARGET_CLASS   TARGET_SUBCLASS   Included .mk       Remarks
# -------------  ---------------  -----------------  -----------------------------------
# APPLICATION    CONSOLE (def)    appsdos.mk         Standard DOS application
# LIBRARY        STATIC (default) libsdos.mk         Static library
# LIBRARY        DYNAMIC          ERROR              Not supported on DOS
# DRIVER         (any)            ERROR (reserved)   Single DOS driver type, not yet implemented

!ifeq TARGET_CLASS APPLICATION
!ifneq TARGET_SUBCLASS CONSOLE
!error TARGET_SUBCLASS=$(TARGET_SUBCLASS) is not valid for DOS APPLICATION. Only CONSOLE allowed.
!endif
TARGET_MK=appsdos
!else ifeq TARGET_CLASS LIBRARY
!ifeq TARGET_SUBCLASS STATIC
TARGETS  = $(PATH)$(PROJ).lib
TARGET_MK=libsdos

!else ifeq TARGET_SUBCLASS DYNAMIC
!error LIBRARY DYNAMIC is not supported on DOS
!else
!error Unknown TARGET_SUBCLASS for DOS LIBRARY: $(TARGET_SUBCLASS)
!endif
!else ifeq TARGET_CLASS DRIVER
!error DRIVER class is reserved and not yet implemented for DOS
!else
!error Unknown TARGET_CLASS for DOS: $(TARGET_CLASS)
!endif

# --- DPMI ---
!else ifeq TARGET_API DPMI
# ============================================================
# DPMI classes (TARGET_API=DPMI)
# ============================================================
# TARGET_CLASS   TARGET_SUBCLASS   Included .mk       Remarks
# -------------  ---------------  -----------------  -----------------------------------
# APPLICATION    CONSOLE (def)    appsdos.mk         DPMI application
# LIBRARY        DYNAMIC (default) ERROR (reserved)   Not yet implemented
# LIBRARY        STATIC           libsdos.mk         Static library
# DRIVER         any              ERROR              Not supported on DPMI

!ifeq TARGET_CLASS APPLICATION
!ifneq TARGET_SUBCLASS CONSOLE
!error TARGET_SUBCLASS=$(TARGET_SUBCLASS) is not valid for DPMI APPLICATION. Only CONSOLE allowed.
!endif
TARGET_MK=appsdos
!else ifeq TARGET_CLASS LIBRARY
!ifeq TARGET_SUBCLASS DYNAMIC
!error LIBRARY DYNAMIC is reserved and not yet implemented for DPMI
!else ifeq TARGET_SUBCLASS STATIC
TARGET_MK=libsdos
!else
!error Unknown TARGET_SUBCLASS for DPMI LIBRARY: $(TARGET_SUBCLASS)
!endif
!else ifeq TARGET_CLASS DRIVER
!error DRIVER class is not supported on DPMI
!else
!error Unknown TARGET_CLASS for DPMI: $(TARGET_CLASS)
!endif

# --- HOST ---
!else ifeq TARGET_API HOST

# ============================================================
# HOST classes (TARGET_API=HOST)
# ============================================================
# TARGET_CLASS   TARGET_SUBCLASS   Included .mk       Remarks
# -------------  ---------------  -----------------  -----------------------------------
# APPLICATION    CONSOLE (def)    tools.mk           Host tool (TARGET_LANG pascal: toolspas.mk)
# LIBRARY        DYNAMIC          tools.mk           DLL (DLL=1)
# LIBRARY        STATIC (default) libs.mk            Static library
# DRIVER         any              ERROR              Not supported on HOST

!ifeq TARGET_CLASS APPLICATION
!ifeq TARGET_LANG pascal
TARGET_MK=toolspas
!else

!ifdef LIBS
pth=$$(pth)
!ifndef ADDLIBS
ADDLIBS = $(pth)$(LIBS: =.lib $(pth)).lib
!else
ADDLIBS = $(ADDLIBS) $(pth)$(LIBS: =.lib $(pth)).lib
!endif
pth=$(%ROOT)build$(SEP)lib$(SEP)
ADD_LINKOPT = $(ADD_LINKOPT) lib $(LIBS: =.lib lib ).lib
!endif

TARGET_MK=tools
!endif
!else ifeq TARGET_CLASS LIBRARY
!ifeq TARGET_SUBCLASS DYNAMIC
DLL = 1
TARGET_MK=tools
!else ifeq TARGET_SUBCLASS STATIC


##############################
TRGT = $(PROJ).lib

!ifdef LIBS
pth=$$(pth)
!ifndef ADDLIBS
ADDLIBS = $(pth)$(LIBS: =.lib $(pth)).lib
!else
ADDLIBS = $(ADDLIBS) $(pth)$(LIBS: =.lib $(pth)).lib
!endif
pth=$(%ROOT)build$(SEP)lib$(SEP)
!endif

TARGET_MK=libs

##############################

!endif
!else ifeq TARGET_CLASS DRIVER
!error DRIVER class is not supported on HOST
!else
!error Unknown TARGET_CLASS for HOST: $(TARGET_CLASS)
!endif


# --- WIN ---
!else ifeq TARGET_API WIN

# ============================================================
# WIN16 classes (16-bit Windows)
# ============================================================
# TARGET_CLASS   TARGET_SUBCLASS   Included .mk       Remarks
# -------------  ---------------  -----------------  -----------------------------------
# APPLICATION    GUI (default)    appsw16.mk         Standard GUI application
# APPLICATION    HYBRID           appshybrid.mk      DOS stub + Win16 GUI (diff sources)
# APPLICATION    CONSOLE          ERROR              Not supported on Win16
# APPLICATION    FAMILY           ERROR              Not supported on Win16
# APPLICATION    DUAL             ERROR              Not supported on Win16
# LIBRARY        DYNAMIC          appsw16.mk         DLL (DLL=1)
# LIBRARY        STATIC           libsw16.mk         Static library
# DRIVER         PHYSICAL         appsw16.mk         Physical driver DLL (DLL=1)
# DRIVER         VIRTUAL          ERROR (reserved)   Virtual driver (VxD)

!ifeq TARGET_BITS 16

# Require add SPDX tags
#LICENSE_CHECK = 1

!ifeq TARGET_VERSION 300
ADD_RCOPT = -30
!endif

!ifeq TARGET_VERSION 310
ADD_RCOPT = -31
ADD_LINKOPT=lib commdlg.lib lib shell.lib lib lzexpand.lib
!endif

ADD_RCOPT = $(ADD_RCOPT) -bt=windows -i=. -i=$(WATCOM)$(SEP)h$(SEP)win

!ifeq TARGET_CLASS APPLICATION

ADD_COPT = $(ADD_COPT) -sg
!ifeq TARGET_VERSION 310
!endif

# LIBS -> ADD_LINKOPT
!ifdef LIBS
ADD_LINKOPT = $(ADD_LINKOPT) lib $(LIBS: =.lib lib ).lib
!endif
!ifeq TARGET_SUBCLASS GUI
TARGET_MK=appsw16
!else ifeq TARGET_SUBCLASS HYBRID
TARGET_MK=appshybrid
!else ifeq TARGET_SUBCLASS CONSOLE
!error CONSOLE is not supported on 16-bit Windows
!else ifeq TARGET_SUBCLASS FAMILY
!error FAMILY is not supported on 16-bit Windows
!else ifeq TARGET_SUBCLASS DUAL
!error DUAL is not supported on 16-bit Windows
!else
!error Unknown TARGET_SUBCLASS for 16-bit Windows APPLICATION: $(TARGET_SUBCLASS)
!endif

!else ifeq TARGET_CLASS LIBRARY

!ifeq TARGET_SUBCLASS DYNAMIC
DLL = 1
# LIBS -> ADD_LINKOPT

!ifdef LIBS
pth=$$(pth)
!ifndef ADDLIBS
ADDLIBS = $(pth)$(LIBS: =.lib $(pth)).lib
!else
ADDLIBS = $(ADDLIBS) $(pth)$(LIBS: =.lib $(pth)).lib
!endif
pth=$(%ROOT)build$(SEP)lib$(SEP)
ADD_LINKOPT = $(ADD_LINKOPT) lib $(LIBS: =.lib lib ).lib
!endif

TARGET_MK=appsw16

!else ifeq TARGET_SUBCLASS STATIC
TRGT = $(PROJ).lib

!ifdef LIBS
pth=$$(pth)
!ifndef ADDLIBS
ADDLIBS = $(pth)$(LIBS: =.lib $(pth)).lib
!else
ADDLIBS = $(ADDLIBS) $(pth)$(LIBS: =.lib $(pth)).lib
!endif
pth=$(%ROOT)build$(SEP)lib$(SEP)
!endif

TARGET_MK=libsw16

!else
!error Unknown TARGET_SUBCLASS for 16-bit Windows LIBRARY: $(TARGET_SUBCLASS)
!endif

!else ifeq TARGET_CLASS DRIVER

!ifeq TARGET_SUBCLASS PHYSICAL
DLL = 1
TARGET_MK=appsw16
!else ifeq TARGET_SUBCLASS VIRTUAL
!error DRIVER VIRTUAL is reserved and not yet implemented for 16-bit Windows
!else
!error Unknown TARGET_SUBCLASS for 16-bit Windows DRIVER: $(TARGET_SUBCLASS). Supported: PHYSICAL, VIRTUAL(reserved).
!endif

!else
!error Unknown TARGET_CLASS for 16-bit Windows: $(TARGET_CLASS)
!endif

!else ifeq TARGET_BITS 32
# ============================================================
# WIN32 classes (32-bit Windows)
# ============================================================
# TARGET_CLASS   TARGET_SUBCLASS   Included .mk       Remarks
# -------------  ---------------  -----------------  -----------------------------------
# APPLICATION    GUI (default)    appsw32.mk         GUI application (WIN_GUI=1)
# APPLICATION    CONSOLE          appsw32.mk         Console application (WIN_CONSOLE=1)
# APPLICATION    FAMILY           ERROR (reserved)
# APPLICATION    DUAL             ERROR (reserved)
# APPLICATION    HYBRID           ERROR (reserved)
# LIBRARY        DYNAMIC          appsw16.mk         DLL (DLL=1)
# LIBRARY        STATIC           ERROR (reserved)
# DRIVER         any              ERROR (reserved)

!ifeq TARGET_CLASS APPLICATION

!ifeq TARGET_SUBCLASS GUI
WIN_GUI = 1
TARGET_MK=appsw32
!else ifeq TARGET_SUBCLASS CONSOLE
WIN_CONSOLE = 1
TARGET_MK=appsw32
!else ifeq TARGET_SUBCLASS FAMILY
!error FAMILY for Win32 is reserved and not yet implemented
!else ifeq TARGET_SUBCLASS DUAL
!error DUAL for Win32 is reserved and not yet implemented
!else ifeq TARGET_SUBCLASS HYBRID
!error HYBRID for Win32 is reserved and not yet implemented
!else
!error Unknown TARGET_SUBCLASS for 32-bit Windows APPLICATION: $(TARGET_SUBCLASS)
!endif

!else ifeq TARGET_CLASS LIBRARY

!ifeq TARGET_SUBCLASS DYNAMIC
DLL = 1
TARGET_MK=appsw16
!else ifeq TARGET_SUBCLASS STATIC
!error LIBRARY STATIC is reserved and not yet implemented for 32-bit Windows
!else
!error Unknown TARGET_SUBCLASS for 32-bit Windows LIBRARY: $(TARGET_SUBCLASS)
!endif

!else ifeq TARGET_CLASS DRIVER

!error DRIVER class is reserved and not yet implemented for 32-bit Windows

!else

!error Unknown TARGET_CLASS for 32-bit Windows: $(TARGET_CLASS)

!endif

!else
!error Unsupported TARGET_BITS for WIN: $(TARGET_BITS)
!endif

# --- OS/2 ---
!else ifeq TARGET_API OS2

# ============================================================
# OS2_16 classes (16-bit OS/2)
# ============================================================
# TARGET_CLASS   TARGET_SUBCLASS   Included .mk       Remarks
# -------------  ---------------  -----------------  -----------------------------------
# APPLICATION    CONSOLE (def)    appsos2v1.mk       Console application
# APPLICATION    GUI              appsos2v1.mk       Presentation Manager (PM=1)
# APPLICATION    FAMILY           appsfapi.mk        Family app (NE exe + DOS stub via BIND)
# APPLICATION    DUAL             appsdual.mk        DOS stub + OS/2 16-bit (same sources)
# APPLICATION    HYBRID           appshybrid.mk      DOS stub + OS/2 16-bit (diff sources)
# LIBRARY        DYNAMIC          appsos2v1.mk       DLL (DLL=1)
# LIBRARY        STATIC           ERROR (reserved)
# DRIVER         PHYSICAL         appsos2v1.mk       Physical device driver (PHYSDEVICE=1)
# DRIVER         VIRTUAL          appsos2v1.mk       Virtual device driver (VIRTDEVICE=1)

!ifeq TARGET_BITS 16

!ifeq TARGET_CLASS APPLICATION

!ifeq TARGET_SUBCLASS CONSOLE
TARGET_MK=appsos2v1
!else ifeq TARGET_SUBCLASS GUI
PM = 1
TARGET_MK=appsos2v1
!else ifeq TARGET_SUBCLASS FAMILY
TARGET_MK=appsfapi
!else ifeq TARGET_SUBCLASS DUAL
TARGET_MK=appsdual
!else ifeq TARGET_SUBCLASS HYBRID
TARGET_MK=appshybrid
!else
!error Unknown TARGET_SUBCLASS for 16-bit OS/2 APPLICATION: $(TARGET_SUBCLASS)
!endif

!else ifeq TARGET_CLASS LIBRARY

!ifeq TARGET_SUBCLASS DYNAMIC
DLL = 1
TARGET_MK=appsos2v1
!else ifeq TARGET_SUBCLASS STATIC
!error LIBRARY STATIC is reserved and not yet implemented for 16-bit OS/2
!else
!error Unknown TARGET_SUBCLASS for 16-bit OS/2 LIBRARY: $(TARGET_SUBCLASS)
!endif

!else ifeq TARGET_CLASS DRIVER

!ifeq TARGET_SUBCLASS PHYSICAL
PHYSDEVICE = 1
TARGET_MK=appsos2v1
!else ifeq TARGET_SUBCLASS VIRTUAL
VIRTDEVICE = 1
TARGET_MK=appsos2v1
!else
!error Unknown TARGET_SUBCLASS for 16-bit OS/2 DRIVER: $(TARGET_SUBCLASS). Supported: PHYSICAL, VIRTUAL.
!endif

!else

!error Unknown TARGET_CLASS for 16-bit OS/2: $(TARGET_CLASS)

!endif

!else ifeq TARGET_BITS 32
# ============================================================
# OS2_32 classes (32-bit OS/2)
# ============================================================
# TARGET_CLASS   TARGET_SUBCLASS   Included .mk       Remarks
# -------------  ---------------  -----------------  -----------------------------------
# APPLICATION    CONSOLE (def)    appsos2.mk         Console application
# APPLICATION    GUI              appsos2.mk         Presentation Manager (PM=1)
# APPLICATION    FAMILY           ERROR (reserved)
# APPLICATION    DUAL             appsdual.mk        DOS stub + OS/2 32-bit (same sources)
# APPLICATION    HYBRID           appshybrid.mk      DOS stub + OS/2 32-bit (diff sources)
# LIBRARY        DYNAMIC          appsos2.mk         DLL (DLL=1)
# LIBRARY        STATIC           libsos2.mk
# DRIVER         PHYSICAL         appsos2.mk         Physical device driver (PHYSDEVICE=1)
# DRIVER         VIRTUAL          appsos2.mk         Virtual device driver (VIRTDEVICE=1)

!ifeq TARGET_CLASS APPLICATION

!ifdef LIBS
ADD_LINKOPT = $(ADD_LINKOPT) lib $(LIBS: =.lib lib ).lib
!endif


!ifeq TARGET_SUBCLASS CONSOLE
TARGET_MK=appsos2
!else ifeq TARGET_SUBCLASS GUI
PM = 1
TARGET_MK=appsos2
!else ifeq TARGET_SUBCLASS FAMILY
!error FAMILY for 32-bit OS/2 is reserved and not yet implemented
!else ifeq TARGET_SUBCLASS DUAL
TARGET_MK=appsdual
!else ifeq TARGET_SUBCLASS HYBRID
TARGET_MK=appshybrid
!else
!error Unknown TARGET_SUBCLASS for 32-bit OS/2 APPLICATION: $(TARGET_SUBCLASS)
!endif

!else ifeq TARGET_CLASS LIBRARY

!ifeq TARGET_SUBCLASS DYNAMIC
DLL = 1
TARGET_MK=appsos2
!else ifeq TARGET_SUBCLASS STATIC

TRGT = $(PROJ).lib

!ifdef LIBS
pth=$$(pth)
!ifndef ADDLIBS
ADDLIBS = $(pth)$(LIBS: =.lib $(pth)).lib
!else
ADDLIBS = $(ADDLIBS) $(pth)$(LIBS: =.lib $(pth)).lib
!endif
pth=$(%ROOT)build$(SEP)lib$(SEP)
!endif

TARGET_MK=libsos2
!else
!error Unknown TARGET_SUBCLASS for 32-bit OS/2 LIBRARY: $(TARGET_SUBCLASS)
!endif

!else ifeq TARGET_CLASS DRIVER

!ifeq TARGET_SUBCLASS PHYSICAL
PHYSDEVICE = 1
TARGET_MK=appsos2
!else ifeq TARGET_SUBCLASS VIRTUAL
VIRTDEVICE = 1
TARGET_MK=appsos2
!else
!error Unknown TARGET_SUBCLASS for 32-bit OS/2 DRIVER: $(TARGET_SUBCLASS). Supported: PHYSICAL, VIRTUAL.
!endif

!else

!error Unknown TARGET_CLASS for 32-bit OS/2: $(TARGET_CLASS)

!endif

!else
!error Unsupported TARGET_BITS for OS2: $(TARGET_BITS)
!endif


!else
!error Unknown TARGET_API: $(TARGET_API)
!endif

!include $(%ROOT)tools/mk/$(TARGET_MK).mk

!endif
