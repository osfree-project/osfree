# tools/mk/build_setuppkg.mk
# Setup disk builder for osFree build system.
#
# Called when TARGET_API=PACKAGE, TARGET_CLASS=SETUP.
# Requires: PROJ, FILESDIR, BLD
#
# Reads:   $(BLD)packages/$(PROJ).map
# Produces: files staged in $(FILESDIR)pkgs/$(PROJ)/
#
# If setup.inf / setup.exe are present in the package source
# directory, they are copied to staging.

!ifndef __build_setuppkg_mk__
!define __build_setuppkg_mk__

!include $(%ROOT)tools/mk/dirs.mk

# ------------------------------------------------------------
# Minimal command set (subset of all.mk)
# ------------------------------------------------------------
!ifeq UNIX FALSE
CP  = copy
SAY = echo
!else
CP  = cp
SAY = echo
!endif

verbose = $(%VERBOSE)
!ifeq verbose yes
verbose =
!else
verbose = @
!endif

# ------------------------------------------------------------
# Paths
# ------------------------------------------------------------
STAGING  = $(FILESDIR)pkgs$(SEP)$(PROJ)
REGISTRY = $(BLD)packages$(SEP)$(PROJ).map

# ------------------------------------------------------------
# Two-pass build.
#
# Pass 1 (pkgfile undefined):
#   - ensure $(BLD)packages, registry file, pkgs root and staging exist
#   - recurse into wmake with pkgfile=..., target setuppkg_inner
#
# Pass 2 (pkgfile set):
#   - !include the registry, defines PKG_TARGETS and copy rules
#   - setuppkg_inner depends on $(PKG_TARGETS), so all copies run first
#   - then setup.inf/setup.exe are copied from $(MYDIR) if present
# ------------------------------------------------------------
all: .SYMBOLIC
    @$(MDHIER) $(BLD)packages
    @if not exist $(REGISTRY) @%create $(REGISTRY)
    @$(MDHIER) $(FILESDIR)pkgs
    @$(MDHIER) $(STAGING)
    @$(MAKE) $(MAKEOPT) pkgfile=$(REGISTRY) setuppkg_inner

!ifdef pkgfile
!include $(pkgfile)
!endif

!ifdef pkgfile
!ifndef PKG_TARGETS
!error No components registered in package $(PROJ). Registry: $(pkgfile)
!endif

setuppkg_inner: $(PKG_TARGETS) .SYMBOLIC
    @if exist $(MYDIR)setup.inf $(CP) $(MYDIR)setup.inf $(STAGING)
    @$(SAY) SETUP    $(PROJ) ready
!endif

!endif
