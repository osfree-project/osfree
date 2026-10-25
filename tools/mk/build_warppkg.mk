# tools/mk/build_warppkg.mk
# WarpIn package builder for osFree build system.
#
# Called when TARGET_API=PACKAGE, TARGET_CLASS=WARPIN.
#
# Reads:   $(BLD)packages/$(PROJ).map   (defines pkg_all_files: ...)
# Writes:  $(FILESDIR)pkgs/$(PROJ).wpi
#
# Staging is populated by the map's $(CP) rules (files stay
# uncompressed), then wic packs the whole STAGING tree into .wpi with
# internal compression.

!ifndef __build_warppkg_mk__
!define __build_warppkg_mk__

DEST = none

!include $(%ROOT)tools/mk/dirs.mk
!include $(%ROOT)tools/mk/all.mk

STAGING  = $(FILESDIR)pkgs$(SEP)$(PROJ)
REGISTRY = $(BLD)packages$(SEP)$(PROJ).map
WPI      = $(FILESDIR)pkgs$(SEP)$(PROJ).wpi

!ifndef WIS_FILE
WIS_FILE = $(PROJ).wis
!endif

!ifeq UNIX FALSE
RMDIR = rmdir /s /q
!else
RMDIR = rm -rf
!endif

TARGETS =

prereq: pkgprep

pkgprep: .SYMBOLIC
    @$(MDHIER) $(BLD)packages
    @if not exist $(REGISTRY) @%create $(REGISTRY)
    @$(MDHIER) $(FILESDIR)pkgs
    @$(MDHIER) $(STAGING)
!ifndef pkgfile
    @$(MAKE) $(MAKEOPT) pkgfile=$(REGISTRY) stagedir
!endif

!ifdef pkgfile
!include $(pkgfile)

stagedir: pkg_all_files .SYMBOLIC
    @$(SAY) WPI      $(PROJ).wpi
    $(verbose)wic $(WPI) 1 -c$(STAGING) -r * -s $(MYDIR)$(WIS_FILE)
!endif

clean: pkgclean .symbolic

pkgclean: .SYMBOLIC
    @$(SAY) CLEAN    package $(PROJ)
    $(verbose)if exist $(STAGING) $(verbose)$(RMDIR) $(STAGING) $(BLACKHOLE)
    $(verbose)if exist $(REGISTRY) $(verbose)$(DC) $(REGISTRY) $(BLACKHOLE)
    $(verbose)if exist $(WPI) $(verbose)$(DC) $(WPI) $(BLACKHOLE)

!endif
