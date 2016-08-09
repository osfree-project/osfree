PKGDIR		?= ../../..
L4DIR		?= $(PKGDIR)/../..

include $(PKGDIR)/mk/settings.mk

TARGET		= libhandle.a
MODE		= l4env
SYSTEMS		 = x86-l4v2 arm-l4v2 amd64-l4v2
SRC_C		= handlemgr.c
LIBS		= 
LDFLAGS		= 
CFLAGS          += -I$(PKGDIR)/include

# for debugging don't edit this file buf create ./Makeconf.local with DEBUG=1
# be aware that enabling DEBUG here depends on DEBUG in ldso too because of
# mmap_list_regions()
DEBUG		 ?= 0

include $(L4DIR)/mk/lib.mk

ifneq ($(SYSTEM),)
# additionally to linking TARGET to $(L4DIR)/lib (needed for linking at
# build time), copy TARGET to the binary directory (needed for linking at
# runtime)
BINTARGET	?= $(DROPS_STDDIR)/bin/$(subst -,/,$(SYSTEM)/$(TARGET))
BINTARGET_LOCAL	?= $(OBJ_BASE)/bin/$(subst -,/,$(SYSTEM)/$(TARGET))
INSTALLFILE_BIN_LOCAL ?= cp $(1) $(2) && chmod 644 $(2)

all:: $(BINTARGET_LOCAL)
install:: $(BINTARGET)

$(BINTARGET) $(BINTARGET_LOCAL): $(TARGET)
	@$(INSTALL_LOCAL_MESSAGE)
	$(if $(INSTALLFILE_BIN_LOCAL),$(VERBOSE)$(INSTALL) -d $(dir $@))
	$(VERBOSE)$(call INSTALLFILE_BIN_LOCAL,$<,$@)

.PHONY: $(BINTARGET)
endif
