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

STAGING  = $(FILESDIR)pkgs$(SEP)$(PROJ)$(SEP)
REGISTRY = $(BLD)packages$(SEP)$(PROJ).map

!ifeq UNIX FALSE
RMDIR = rmdir /s /q
!else
RMDIR = rm -rf
!endif

!ifndef DONT_COMPRESS
DONT_COMPRESS =
!endif

# for SETUP: CP means compress, COPY keeps the original copy
COPY =$+$(CP)$-
CP   =$(COMPRESS)
CPMSG = COMPRESS

TARGETS =

prereq: pkgprep

pkgprep: .SYMBOLIC
    $(verbose)$(MDHIER) $(BLD)packages
    $(verbose)if not exist $(REGISTRY) @%create $(REGISTRY)
    $(verbose)$(MDHIER) $(FILESDIR)pkgs
    $(verbose)$(MDHIER) $(STAGING)
!ifndef pkgfile
    $(verbose)$(MAKE) $(MAKEOPT) pkgfile=$(REGISTRY) stagedir
!endif

!ifdef pkgfile
!include $(pkgfile)

stagedir: $(PKG_TARGETS) .SYMBOLIC
    $(verbose)if exist $(MYDIR)setup.inf $(verbose)$(SAY) CP       setup.inf $(LOG)
    $(verbose)if exist $(MYDIR)setup.inf $(verbose)$(COPY) $(MYDIR)setup.inf $(STAGING) $(BLACKHOLE)
!ifdef DONT_COMPRESS
    @for %f in ($(DONT_COMPRESS)) do @$(MAKE) $(MAKEOPT) pkgfile=$(pkgfile) asis=%f asis_one
!endif
    $(verbose)$(SAY) SETUP    $(PROJ) ready


asis_one: .SYMBOLIC
_asis_key1 = $(asis:.=_)
_asis_key2 = $(_asis_key1:-=_)
    $(verbose)if exist "$(pkg_$(_asis_key2)_src)" $(verbose)$(SAY) CP       $(asis) $(LOG)
    $(verbose)if exist "$(pkg_$(_asis_key2)_src)" $(verbose)$(COPY) "$(pkg_$(_asis_key2)_src)" "$(STAGING)$(asis)" $(BLACKHOLE)

clean: pkgclean .symbolic

pkgclean: .SYMBOLIC
    $(verbose)$(SAY) CLEAN    package $(PROJ)
    $(verbose)if exist $(STAGING) $(verbose)$(RMDIR) $(STAGING) $(BLACKHOLE)
    $(verbose)if exist $(REGISTRY) $(verbose)$(DC) $(REGISTRY) $(BLACKHOLE)

!endif

!endif
