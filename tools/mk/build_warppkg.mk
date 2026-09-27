# tools/mk/build_warppkg.mk
# WarpIn package builder for osFree build system.
#
# Called when TARGET_API=PACKAGE, TARGET_CLASS=WARPIN.
# Requires: PROJ, FILESDIR, BLD
# Optional: WIS_FILE (default: $(PROJ).wis)
#
# Reads:  $(BLD)packages/$(PROJ).map
# Writes: $(PROJ).wpi in current directory

!ifndef __build_warppkg_mk__
!define __build_warppkg_mk__

!include $(%ROOT)tools/mk/dirs.mk

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

STAGING  = $(FILESDIR)pkgs$(SEP)$(PROJ)
REGISTRY = $(BLD)packages$(SEP)$(PROJ).map

!ifndef WIS_FILE
WIS_FILE = $(PROJ).wis
!endif

all: .SYMBOLIC
    @$(MDHIER) $(BLD)packages
    @if not exist $(REGISTRY) @%create $(REGISTRY)
    @$(MDHIER) $(FILESDIR)pkgs
    @$(MDHIER) $(STAGING)
    @$(MAKE) $(MAKEOPT) pkgfile=$(REGISTRY) wpi_inner

!ifdef pkgfile
!include $(pkgfile)
!endif

!ifdef pkgfile
!ifndef PKG_TARGETS
!error No components registered in package $(PROJ). Registry: $(pkgfile)
!endif

wpi_inner: $(PKG_TARGETS) .SYMBOLIC
    @$(SAY) WPI      $(PROJ).wpi
    $(verbose)wic $(PROJ).wpi 1 -c$(STAGING) -r * -s $(WIS_FILE)
!endif

!endif
