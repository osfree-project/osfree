PKGDIR		?= ../../..
L4DIR		?= $(PKGDIR)/../..

TARGET		= liblx.s.so
TRG             = lx.ixf
MODE		= l4env
SYSTEMS		= x86-l4v2
SRC_C		= lx.c mod.c load.c loadobj.c fixup.c debug.c l4alloc_mem.c
CFLAGS          = -I$(PKGDIR)/include

NO_DEFAULT_RELOC= 1
LDFLAGS		= --dynamic-linker libld-l4.s.so \
		  --export-dynamic \
		  --rpath-link $(OBJ_BASE)/lib/$(ARCH)_$(CPU)/$(L4API)
LDSCRIPT	= $(call findfile,main_dyn.ld,$(L4LIBDIR))
LIBS		= -Bdynamic -lmemmgr.s -ldl.s \
		  -Bstatic -ll4sys -ll4sys.p

include $(L4DIR)/mk/lib.mk

ifneq ($(SYSTEM),)
# additionally to linking TARGET to $(L4DIR)/lib (needed for linking at
# build time), copy TARGET to the binary directory (needed for linking at
# runtime)
BINTARGET	?= $(DROPS_STDDIR)/bin/$(subst -,/,$(SYSTEM)/$(TARGET))
BINTARGET_LOCAL	?= $(OBJ_BASE)/bin/$(subst -,/,$(SYSTEM)/$(TARGET))
BINTRG         	?= $(DROPS_STDDIR)/bin/$(subst -,/,$(SYSTEM)/$(TARGET))
BINTRG_LOCAL    ?= $(OBJ_BASE)/bin/$(subst -,/,$(SYSTEM)/$(TRG))
INSTALLFILE_BIN_LOCAL ?= cp $(1) $(2) && chmod 644 $(2)

all:: $(BINTRG_LOCAL)
install:: $(BINTRG)

$(BINTRG_LOCAL): $(BINTARGET_LOCAL)
	cp -f $< $@

$(BINTARGET) $(BINTARGET_LOCAL): $(TARGET)
	@$(INSTALL_LOCAL_MESSAGE)
	$(if $(INSTALLFILE_BIN_LOCAL),$(VERBOSE)$(INSTALL) -d $(dir $@))
	$(VERBOSE)$(call INSTALLFILE_BIN_LOCAL,$<,$@)

.PHONY: $(BINTARGET)
endif
