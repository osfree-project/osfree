# tools/mk/build3.mk
# Universal target profile dispatcher for osFree build system
# Version: hierarchical class/subclass model
#
# ============================================================
# STAGE 3: Targets dispatcher
# ============================================================

!ifndef __build3_mk__
!define __build3_mk__

# ------------------------------------------------------------
# Use gen_*_wrapper for *.obj dependencies generation
# (only if SOURCES exists)
# ------------------------------------------------------------
!ifdef SOURCES
WRAPPERS = 1
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

# ============================================================
# Turn on line numbers debug info for ASM (for .AUTODEPEND support)
# ============================================================
!ifeq JWASM 1
ADD_ASMOPT=$(ADD_ASMOPT) -Zd
!else
ADD_ASMOPT=$(ADD_ASMOPT) -d1
!endif

!include $(%ROOT)tools/mk/build_dispatcher.mk

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
TT2=1
!ifdef trrgt
TT=$(trrgt:.=_)
TT2=$(TT:-=_)
!endif
!ifndef $(TT2)
 @$(SAY) Registering project $(trrgt)...
 @%append $(BLD)projects.map $(TT2)=$(deps)
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

# .rc attachment (via _postbuild_res_target) applies only to targets that
# can carry resources — exe/dll. Static libraries never do, and adding
# _res to TARGETS leaks into `library=$(TARGETS)` and breaks wlib.
_res_applies = 1
!ifeq TARGET_CLASS LIBRARY
!ifeq TARGET_SUBCLASS STATIC
_res_applies = 0
!endif
!endif

# For standard extension, insert a dummy target BEFORE the real one
!ifeq _gen_std 1
!ifeq _res_applies 1
_postbuild_res_target = $(PATH)$(PROJ)_res
TARGETS += $(_postbuild_res_target)
$(_postbuild_res_target): .SYMBOLIC
!endif
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
#        @echo GDR_DEBUG PLATFORM=[$(PLATFORM)] PATH=[$(PATH)]
!ifdef pmap
!include $(BLD)projects.map
!endif
!ifdef mpth
# "if exist@ can be removed later after global defs generation
        @if exist $(BLD)$(PLATFORM)$($(mpth)_lib)_deps.mk @%append $(mf) !include $(BLD)$(PLATFORM)$($(mpth)_lib)_deps.mk
!endif

gen_dep_lib: .symbolic
!ifdef pmap
!include $(BLD)projects.map
!endif
        @%append $(PATH)_deps.mk !include $(BLD)$(PLATFORM)$($(pth)_lib)_deps.mk

gen_dep_obj: .symbolic
        @%append $(PATH)_deps.mk $(trgt): $(MYDIR)makefile .AUTODEPEND
	@%append $(PATH)_deps.mk    @$(CD) $(PATH) && $(MAKE) $(__MAKEOPTS__) && cd $(CWD)

gen_deps_wrapper: .symbolic
#        @echo GDW_DEBUG PLATFORM=[$(PLATFORM)] PATH=[$(PATH)]
        # register project
        @if not exist $(BLD)projects.map @%create $(BLD)projects.map
        @$(MAKE) $(MAKEOPT) trrgt=$(TRGT:$(PATH)=) deps=$(RELDIR) pmap=$(BLD)projects.map gen_register_project
        # add to generated makefile RES compile rule
        @if exist $(MYDIR)$(PROJ).rc @$(MAKE) $(MAKEOPT) gen_wrc_rule
        # add to generated makefile OBJS dependencies
        @for %o in ($(OBJS)) do @$(MAKE) $(MAKEOPT) trgt="%o" deps="$(MYDIR)makefile .AUTODEPEND" gen_deps #&& $(SAY) gen_deps=%o
!ifdef LIBS
        #add LIBS _deps.mk to generated makefile
	@for %l in ($(LIBS)) do @$(MAKE) $(MAKEOPT) trgt="$(BLD)lib\%l.lib" mpth="%l" pmap=$(BLD)projects.map gen_dep_rule
!endif
        # generate _deps.mk to be included by other projects for full dependencies
        @if exist $(PATH)_deps.mk @%quit
        @%create $(PATH)_deps.mk
        @%append $(PATH)_deps.mk !ifndef __$(PROJ)_deps_mk__
        @%append $(PATH)_deps.mk !define __$(PROJ)_deps_mk__
        @%append $(PATH)_deps.mk $(DEST:build$(SEP)bin$(SEP)..$(SEP)..$(SEP)build=build)$(SEP)$(TRGT): $(OBJS) $(ADDLIBS)
#        @%append $(PATH)_deps.mk $(BLD)lib$(SEP)$(TRGT): $(OBJS) $(ADDLIBS)
	@%append $(PATH)_deps.mk    @$(CD) $(PATH) && $(MAKE) $(MAKEOPT) && cd $(CWD)
        @for %o in ($(OBJS)) do @$(MAKE) $(MAKEOPT) trgt="%o" pth=$(pth) gen_dep_obj
!ifdef LIBS
        #add LIBS _deps.mk to generated makefile
#	@for %l in ($(LIBS)) do @$(MAKE) $(MAKEOPT) trgt="$(BLD)lib\%l.lib" mpth="%l" pmap=$(BLD)projects.map gen_dep_rule
        #add LIBS rules to _deps.mk
        @for %l in ($(LIBS)) do @$(MAKE) $(MAKEOPT) trgt="$(BLD)lib\%l.lib" pth="%l" pmap=$(BLD)projects.map gen_dep_lib
!endif
        @%append $(PATH)_deps.mk !endif

#!ifdef MSGEXT
#	@for %o in ($(OBJS)) do @$(MAKE) $(MAKEOPT) trgt="%o" deps="$(PATH)$(PROJ).inc" gen_deps
#!endif

doxy-lint: .SYMBOLIC
    $(verbose)if exist $(FILESDIR)$(SEP)host$(SEP)$(%HOST)$(SEP)bin$(SEP)doxy-lint.exe $(verbose)doxy-lint.exe $(MYDIR) $(COPT)

!endif  
