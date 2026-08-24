# tools/mk/build.mk
# Universal target profile dispatcher for osFree build system
# Version: hierarchical class/subclass model
#
# ============================================================
# STAGE 3: Targets dispatcher
# ============================================================

# ============================================================
# TARGET_API / TARGET_BITS matrix (top-level dispatch)
# ============================================================
# TARGET_API     16-bit                 32-bit                 64-bit
# -------------  ---------------------  ---------------------  ---------------------
# BARE           ERROR (reserved)       ERROR (reserved)       ERROR (reserved)
# BIOS           ERROR (reserved)       ERROR (reserved)       ERROR (reserved)
# ABIOS          ERROR (reserved)       ERROR (reserved)       ERROR (reserved)
# UEFI           ERROR (reserved)       ERROR (reserved)       ERROR (reserved)
# DOS            --> DOS_CLASS          appsdos.mk             ERROR (reserved)
# DPMI           appsdos.mk (default)   appsdos.mk             ERROR (reserved)
# WIN            --> WIN16_CLASS        --> WIN32_CLASS        ERROR (reserved)
# OS2            --> OS2_16_CLASS       --> OS2_32_CLASS (def) ERROR (reserved)
# HOST           tools.mk               tools.mk               tools.mk
#
# Note: For HOST, default TARGET_BITS is taken from %HOST_BITS%.
#       "default" marks the TARGET_BITS used when not explicitly set.
#       Entries with "--> TABLE" defer to class/subclass sub-tables below.

!ifndef __build3_mk__
!define __build3_mk__

# ============================================================
# Default TARGET_API is a OS/2 API
# ============================================================

!ifndef TARGET_API

#!!!!!!!!!!!!! Тут проблема с DEST!!!!! Его надо определить до dirs.mk... Поэтому
# приходится дублировать макросы

!include $(%ROOT)/tools/mk/site.mk

!ifeq UNIX FALSE
CWD         = $(%cdrive):$(%cwd)$(SEP)
!else
CWD         = $(%cwd)$(SEP)
!endif

ROOT        = $(%ROOT)
BLD         = $(%ROOT)build$(SEP)

RD          = $(CWD:$(%ROOT)=)
RELDIR_PWD  = $(RD:build$(SEP)=)
RELDIR      = $(RELDIR_PWD:host$(SEP)$(%HOST)$(SEP)=)



# Добавляем разделитель в начало, чтобы отличать начало пути
TEST_STR = *$(RELDIR)

# Пытаемся удалить "*DOS\"
STRIPPED = $(TEST_STR:*DOS$(SEP)=)

# Если строка изменилась — значит, путь начинается с DOS
!ifneq TEST_STR $(STRIPPED)

STRIPPED2 = $(TEST_STR:*WIN16$(SEP)=)
!ifneq STRIPED $(STRIPPED2)
TARGET_API=WIN
!else
TARGET_API=DOS
!endif

!endif

# Пытаемся удалить "*OS2\"
STRIPPED = $(TEST_STR:*OS2$(SEP)=)

# Если строка изменилась — значит, путь начинается с OS2
!ifneq TEST_STR $(STRIPPED)
TARGET_API=OS2
!endif

!endif

!ifndef TARGET_API
!error TARGET_API must be set
!endif

# ------------------------------------------------------------
# Use gen_*_wrapper for *.obj dependencies generation
# (only if SOURCES exists)
# ------------------------------------------------------------
!ifdef SOURCES
WRAPPERS = 1
!endif

# ============================================================
# Turn on line numbers debug info for ASM (for .AUTODEPEND support)
# ============================================================
!ifeq JWASM 1
ADD_ASMOPT=$(ADD_ASMOPT) -Zd
!else
ADD_ASMOPT=$(ADD_ASMOPT) -d1
!endif

# ============================================================
# Default TARGET_BITS per TARGET_API
# ============================================================
# TARGET_API     Default bits   Allowed bits
# -------------  -------------  -----------------
# DOS            16             16, 32
# DPMI           16             16, 32
# WIN            16             16, 32, 64(reserved)
# OS2            32             16, 32
# HOST           (from env)    16, 32, 64
# BARE / BIOS    ERROR (reserved)
!ifndef TARGET_BITS
!ifeq TARGET_API DOS
TARGET_BITS = 16
!else ifeq TARGET_API DPMI
TARGET_BITS = 16
!else ifeq TARGET_API WIN
TARGET_BITS = 16
!else ifeq TARGET_API OS2
TARGET_BITS = 32
!else ifeq TARGET_API HOST
!ifdef %HOST_BITS
TARGET_BITS = $(%HOST_BITS)
!else
!error TARGET_BITS must be set for HOST (16, 32, 64) or define HOST_BITS
!endif
!else ifeq TARGET_API BARE
!error BARE target is reserved and not yet implemented
!else ifeq TARGET_API BIOS
!error BIOS target is reserved and not yet implemented
!else
!error Unknown TARGET_API: $(TARGET_API)
!endif
!endif

# ============================================================
# Bitness validation and convenience flags
# ============================================================
# TARGET_BITS    32_BITS     64_BITS
# -------------  ----------  ----------
# 16             0           0
# 32             1           0
# 64             0           1
!ifeq TARGET_BITS 16
32_BITS = 0
64_BITS = 0
!else ifeq TARGET_BITS 32
32_BITS = 1
64_BITS = 0
!else ifeq TARGET_BITS 64
32_BITS = 0
64_BITS = 1
!else
!error Unsupported TARGET_BITS: $(TARGET_BITS) (expected 16, 32 or 64)
!endif

# ============================================================
# Default TARGET_CLASS and TARGET_SUBCLASS
# ============================================================
# TARGET_CLASS    Default TARGET_SUBCLASS
# -------------   ----------------------
# APPLICATION     CONSOLE (WIN: GUI)
# LIBRARY         DYNAMIC (DOS: STATIC)
# DRIVER          (required except DOS: no subtype needed)
!ifndef TARGET_CLASS
TARGET_CLASS = APPLICATION
!endif

!ifndef TARGET_SUBCLASS
!ifeq TARGET_CLASS APPLICATION
!ifeq TARGET_API WIN
TARGET_SUBCLASS = GUI
!else
TARGET_SUBCLASS = CONSOLE
!endif
!else ifeq TARGET_CLASS LIBRARY
!ifeq TARGET_API DOS
TARGET_SUBCLASS = STATIC
!else
TARGET_SUBCLASS = DYNAMIC
!endif
!else ifeq TARGET_CLASS DRIVER
!ifneq TARGET_API DOS
!error TARGET_SUBCLASS is required for DRIVER class on $(TARGET_API)
!endif
!else
!error Unknown TARGET_CLASS: $(TARGET_CLASS). Supported: APPLICATION, LIBRARY, DRIVER.
!endif
!endif

# -------------------------------------------------------------
# Sources -> srcfiles
# -------------------------------------------------------------

!ifdef SOURCES
!ifneq SOURCES
p = $$(p)
e = $$(e)
srcfiles = $(p)$(SOURCES: =$(e) $(p))$(e)
!endif
!endif

# -------------------------------------------------------------
# Define DLL for DLLs
# -------------------------------------------------------------

!ifeq TARGET_CLASS LIBRARY
!ifeq TARGET_SUBCLASS DYNAMIC
DLL = 1
!endif
!endif

# -------------------------------------------------------------
# Define TRGT with extension
# -------------------------------------------------------------
!ifdef TARGET_EXT
TRGT = $(PROJ).$(TARGET_EXT)
!else

!ifdef DLL
TRGT = $(PROJ).dll
!else
TRGT = $(PROJ).exe
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
# LIBRARY        STATIC (default) ERROR (reserved)   Not yet implemented
# LIBRARY        DYNAMIC          ERROR              Not supported on DOS
# DRIVER         (any)            ERROR (reserved)   Single DOS driver type, not yet implemented
!ifeq TARGET_CLASS APPLICATION
!ifneq TARGET_SUBCLASS CONSOLE
!error TARGET_SUBCLASS=$(TARGET_SUBCLASS) is not valid for DOS APPLICATION. Only CONSOLE allowed.
!endif
!include $(%ROOT)tools/mk/appsdos.mk
!else ifeq TARGET_CLASS LIBRARY
!ifeq TARGET_SUBCLASS STATIC
!error LIBRARY STATIC is reserved and not yet implemented for DOS
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
# LIBRARY        STATIC           ERROR (reserved)   Not yet implemented
# DRIVER         any              ERROR              Not supported on DPMI
!ifeq TARGET_CLASS APPLICATION
!ifneq TARGET_SUBCLASS CONSOLE
!error TARGET_SUBCLASS=$(TARGET_SUBCLASS) is not valid for DPMI APPLICATION. Only CONSOLE allowed.
!endif
!include $(%ROOT)tools/mk/appsdos.mk
!else ifeq TARGET_CLASS LIBRARY
!ifeq TARGET_SUBCLASS DYNAMIC
!error LIBRARY DYNAMIC is reserved and not yet implemented for DPMI
!else ifeq TARGET_SUBCLASS STATIC
!error LIBRARY STATIC is reserved and not yet implemented for DPMI
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
!include $(%ROOT)tools/mk/tools.mk

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
# LIBRARY        STATIC           ERROR (reserved)
# DRIVER         PHYSICAL         appsw16.mk         Physical driver DLL (DLL=1)
# DRIVER         VIRTUAL          ERROR (reserved)   Virtual driver (VxD)
!ifeq TARGET_BITS 16

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
!include $(%ROOT)tools/mk/appsw16.mk
!else ifeq TARGET_SUBCLASS HYBRID
!include $(%ROOT)tools/mk/appshybrid.mk
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
!include $(%ROOT)tools/mk/appsw16.mk
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
!include $(%ROOT)tools/mk/libsw16.mk
$(TARGETS): $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(TARGETS) library
!else
!error Unknown TARGET_SUBCLASS for 16-bit Windows LIBRARY: $(TARGET_SUBCLASS)
!endif
!else ifeq TARGET_CLASS DRIVER
!ifeq TARGET_SUBCLASS PHYSICAL
DLL = 1
!include $(%ROOT)tools/mk/appsw16.mk
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
# APPLICATION    GUI (default)    appsw16.mk         GUI application (WIN_GUI=1)
# APPLICATION    CONSOLE          appsw16.mk         Console application (WIN_CONSOLE=1)
# APPLICATION    FAMILY           ERROR (reserved)
# APPLICATION    DUAL             ERROR (reserved)
# APPLICATION    HYBRID           ERROR (reserved)
# LIBRARY        DYNAMIC          appsw16.mk         DLL (DLL=1)
# LIBRARY        STATIC           ERROR (reserved)
# DRIVER         any              ERROR (reserved)

!ifeq TARGET_CLASS APPLICATION
!ifeq TARGET_SUBCLASS GUI
WIN_GUI = 1
!include $(%ROOT)tools/mk/appsw16.mk
!else ifeq TARGET_SUBCLASS CONSOLE
WIN_CONSOLE = 1
!include $(%ROOT)tools/mk/appsw16.mk
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
!include $(%ROOT)tools/mk/appsw16.mk
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
!include $(%ROOT)tools/mk/appsos2v1.mk
!else ifeq TARGET_SUBCLASS GUI
PM = 1
!include $(%ROOT)tools/mk/appsos2v1.mk
!else ifeq TARGET_SUBCLASS FAMILY
!include $(%ROOT)tools/mk/appsfapi.mk
!else ifeq TARGET_SUBCLASS DUAL
!include $(%ROOT)tools/mk/appsdual.mk
!else ifeq TARGET_SUBCLASS HYBRID
!include $(%ROOT)tools/mk/appshybrid.mk
!else
!error Unknown TARGET_SUBCLASS for 16-bit OS/2 APPLICATION: $(TARGET_SUBCLASS)
!endif
!else ifeq TARGET_CLASS LIBRARY
!ifeq TARGET_SUBCLASS DYNAMIC
DLL = 1
!include $(%ROOT)tools/mk/appsos2v1.mk
!else ifeq TARGET_SUBCLASS STATIC
!error LIBRARY STATIC is reserved and not yet implemented for 16-bit OS/2
!else
!error Unknown TARGET_SUBCLASS for 16-bit OS/2 LIBRARY: $(TARGET_SUBCLASS)
!endif
!else ifeq TARGET_CLASS DRIVER
!ifeq TARGET_SUBCLASS PHYSICAL
PHYSDEVICE = 1
!include $(%ROOT)tools/mk/appsos2v1.mk
!else ifeq TARGET_SUBCLASS VIRTUAL
VIRTDEVICE = 1
!include $(%ROOT)tools/mk/appsos2v1.mk
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
# LIBRARY        STATIC           ERROR (reserved)
# DRIVER         PHYSICAL         appsos2.mk         Physical device driver (PHYSDEVICE=1)
# DRIVER         VIRTUAL          appsos2.mk         Virtual device driver (VIRTDEVICE=1)

!ifeq TARGET_CLASS APPLICATION
!ifeq TARGET_SUBCLASS CONSOLE
!include $(%ROOT)tools/mk/appsos2.mk
!else ifeq TARGET_SUBCLASS GUI
PM = 1
!include $(%ROOT)tools/mk/appsos2.mk
!else ifeq TARGET_SUBCLASS FAMILY
!error FAMILY for 32-bit OS/2 is reserved and not yet implemented
!else ifeq TARGET_SUBCLASS DUAL
!include $(%ROOT)tools/mk/appsdual.mk
!else ifeq TARGET_SUBCLASS HYBRID
!include $(%ROOT)tools/mk/appshybrid.mk
!else
!error Unknown TARGET_SUBCLASS for 32-bit OS/2 APPLICATION: $(TARGET_SUBCLASS)
!endif
!else ifeq TARGET_CLASS LIBRARY
!ifeq TARGET_SUBCLASS DYNAMIC
DLL = 1
!include $(%ROOT)tools/mk/appsos2.mk
!else ifeq TARGET_SUBCLASS STATIC
!error LIBRARY STATIC is reserved and not yet implemented for 32-bit OS/2
!else
!error Unknown TARGET_SUBCLASS for 32-bit OS/2 LIBRARY: $(TARGET_SUBCLASS)
!endif
!else ifeq TARGET_CLASS DRIVER
!ifeq TARGET_SUBCLASS PHYSICAL
PHYSDEVICE = 1
!include $(%ROOT)tools/mk/appsos2.mk
!else ifeq TARGET_SUBCLASS VIRTUAL
VIRTDEVICE = 1
!include $(%ROOT)tools/mk/appsos2.mk
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


# ============================================================
# Universal support for non-standard target extension
# ============================================================
!ifdef TARGET_EXT
TARGETS += $(PATH)$(PROJ).$(TARGET_EXT)
!endif


!ifdef TARGET_EXT
_std_ext = exe
!ifdef DLL
_std_ext = dll
!endif

!ifneq TARGET_EXT $(_std_ext)
#$(PATH)$(PROJ).$(TARGET_EXT): $(PATH)$(PROJ)$(_std_ext)
#	@$(SAY) CP       $^. $(LOG)
#	$(verbose)$(CP) $< $@ $(BLACKHOLE)

targets: $(PATH)$(PROJ).$(TARGET_EXT)
!endif
!endif

# ============================================================
# Override gen_deps_wrapper to:
#  - track makefile changes (per-object dependencies)
#  - register current project in projects.map
#  - add dependencies of link script on libraries (via map)
#  - add dependencies of objects on generated .inc (messages)
# ============================================================

# Register project (called from gen_deps_wrapper)

gen_register_project: .SYMBOLIC
!ifdef pmap
!include $(pmap)
!endif
TT=1
!ifdef trrgt
TT=$(trrgt:.=_)
!endif
!ifndef $(TT)
 @$(SAY) Registering project $(trrgt)...
 @%append $(BLD)projects.map $(TT)=$(deps)
!else
 @%null
!endif

# Determine if using standard extension
!ifndef TARGET_EXT
_gen_std = 1
!else ifeq TARGET_EXT $(_std_ext)
_gen_std = 1
!else
_gen_std = 0
!endif

# For standard extension, insert a dummy target BEFORE the real one
!ifeq _gen_std 1
_postbuild_res_target = $(PATH)$(PROJ)_res
TARGETS += $(_postbuild_res_target)
$(_postbuild_res_target): .SYMBOLIC
!endif

# ============================================================
# Resource attachment (automatic, no changes in apps*.mk)
# ============================================================

gen_wrc_rule: .SYMBOLIC
!ifeq _gen_std 1
        @%append $(mf) $(_postbuild_res_target): $(MYDIR)$(PROJ).rc
        @%append $(mf) 	@$(SAY) RESCMP   $(TRGT) $(LOG)
        @%append $(mf) 	$(verbose)$(CP) $(PATH)$(TRGT) $(PATH)$(PROJ).tmp $(BLACKHOLE)
        @%append $(mf) 	$(verbose)$(RC) $(RCOPT) $(MYDIR)$(PROJ).rc $(PATH)$(PROJ).tmp -fe=$(PATH)$(TRGT) -fo=$(PATH)$(TRGT) $(BLACKHOLE)
        @%append $(mf) 	$(verbose)$(DC) $(PATH)$(PROJ).tmp $(BLACKHOLE)
!else
        @%append $(mf) $(PATH)$(TRGT): $(PATH)$(PROJ).$(_std_ext) $(MYDIR)$(PROJ).rc
        @%append $(mf) 	@$(SAY) RESCMP   $(TRGT) $(LOG)
        @%append $(mf) 	$(verbose)$(RC) $(RCOPT) $(MYDIR)$(PROJ).rc $(PATH)$(PROJ).$(_std_ext) -fe=$(PATH)$(TRGT) -fo=$(PATH)$(TRGT)
!endif

gen_dep_rule: .symbolic
!ifdef pmap
!include $(BLD)projects.map
!endif
!ifdef mpth
# "if exist@ can be removed later after global defs generation
        @if exist $(BLD)$($(mpth)_lib)_deps.mk @%append $(mf) !include $(BLD)$($(mpth)_lib)_deps.mk
!endif

gen_dep_lib: .symbolic
!ifdef pmap
!include $(BLD)projects.map
!endif
        @%append $(PATH)_deps.mk !include $(BLD)$($(pth)_lib)_deps.mk

gen_dep_obj: .symbolic
        @%append $(PATH)_deps.mk $(trgt): $(MYDIR)makefile .AUTODEPEND
	@%append $(PATH)_deps.mk    @$(CD) $(PATH) && $(MAKE) -h && cd $(CWD)

gen_deps_wrapper: .symbolic
        @if not exist $(BLD)projects.map @%create $(BLD)projects.map
        @$(MAKE) $(MAKEOPT) trrgt=$(TRGT:$(PATH)=) deps=$(RELDIR) pmap=$(BLD)projects.map gen_register_project
        @if exist $(MYDIR)$(PROJ).rc @$(MAKE) $(MAKEOPT) gen_wrc_rule
        @for %o in ($(OBJS)) do @$(MAKE) $(MAKEOPT) trgt="%o" deps="$(MYDIR)makefile .AUTODEPEND" gen_deps
        @%create $(PATH)_deps.mk
        @%append $(PATH)_deps.mk $(BLD)lib$(SEP)$(TRGT): $(OBJS) $(ADDLIBS)
	@%append $(PATH)_deps.mk    @$(CD) $(PATH) && $(MAKE) -h && cd $(CWD)
        @for %o in ($(OBJS)) do @$(MAKE) $(MAKEOPT) trgt="%o" pth=$(pth) gen_dep_obj
!ifdef LIBS
	@for %l in ($(LIBS)) do @$(MAKE) $(MAKEOPT) trgt="$(BLD)lib\%l.lib" mpth="%l" pmap=$(BLD)projects.map gen_dep_rule
        @for %l in ($(LIBS)) do @$(MAKE) $(MAKEOPT) trgt="$(BLD)lib\%l.lib" pth="%l" pmap=$(BLD)projects.map gen_dep_lib
!endif

#!ifdef MSGEXT
#	@for %o in ($(OBJS)) do @$(MAKE) $(MAKEOPT) trgt="%o" deps="$(PATH)$(PROJ).inc" gen_deps
#!endif


!endif  
