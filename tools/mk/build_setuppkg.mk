# tools/mk/build_setuppkg.mk
# Setup disk builder for osFree build system.
#
# Called when TARGET_API=PACKAGE, TARGET_CLASS=SETUP.
#
# Reads:   $(BLD)packages/$(PROJ).map
# Produces: staging directory $(FILESDIR)pkgs/$(PROJ)/
#
# Map provides (accumulated via +=, with explicit space separator so
# tokens stay short — see build3.mk):
#   PKG_TARGETS=$(PKG_TARGETS) $(STAGING)\<file>
# and per-file rule:
#   $(STAGING)\<file>: <src>
#       $(CP) $< $@
#
# For SETUP, CP is redefined to COMPRESS for this makefile, so the
# per-file rules become compress rules.  The patched compress takes
# exactly (input, output) — no name mangling.
#
# stagedir lists $(PKG_TARGETS) as RULE DEPENDENCIES, not as sub-make
# arguments.  Each target in PKG_TARGETS is short (<259 chars per
# token — that's wmake's per-token limit, not the whole line), so
# wmake builds each one through the map's per-file rule:
#   compress <src> <dst>
# Two arguments per invocation.  Nothing is passed on wmake's command
# line beyond the makefile name, pkgfile and the target `stagedir`.

!ifndef __build_setuppkg_mk__
!define __build_setuppkg_mk__

DEST = none

!include $(%ROOT)tools/mk/dirs.mk
!include $(%ROOT)tools/mk/all.mk

STAGING  = $(FILESDIR)pkgs$(SEP)$(PROJ)
REGISTRY = $(BLD)packages$(SEP)$(PROJ).map

!ifeq UNIX FALSE
RMDIR = rmdir /s /q
!else
RMDIR = rm -rf
!endif

# for SETUP: CP means compress, COPY keeps the original copy
COPY = $(CP)
CP   = $(COMPRESS)

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

stagedir: $(PKG_TARGETS) .SYMBOLIC
    @if exist $(MYDIR)setup.inf $(COPY) $(MYDIR)setup.inf $(STAGING)
    @if exist $(MYDIR)setup.exe $(COPY) $(MYDIR)setup.exe $(STAGING)
    @$(SAY) SETUP    $(PROJ) ready
!endif

clean: pkgclean .symbolic

pkgclean: .SYMBOLIC
    @$(SAY) CLEAN    package $(PROJ)
    $(verbose)if exist $(STAGING) $(verbose)$(RMDIR) $(STAGING) $(BLACKHOLE)
    $(verbose)if exist $(REGISTRY) $(verbose)$(DC) $(REGISTRY) $(BLACKHOLE)

!endif
