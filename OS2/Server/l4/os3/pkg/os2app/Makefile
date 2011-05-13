PKGDIR		?= ../..
L4DIR		?= $(PKGDIR)/../..

TARGET		= os2app
TRG             = kal.map
MODE		= l4env_base
SYSTEMS		= x86-l4v2 # arm-l4v2 amd64-l4v2
SRC_C		= main.c $(PKGDIR)/shared/startup/startup.c \
		  kal/apistub.c
CFLAGS          = -I$(PKGDIR)/include
LDFLAGS         = -Map os2app.mp
LIBS		= -los2exec -los2fs -los2server -lloaderif -levents
DEFAULT_RELOC   = 0xbb000000

include $(L4DIR)/mk/prog.mk

ifneq ($(SYSTEM),)
# additionally to linking TARGET to $(L4DIR)/lib (needed for linking at
# build time), copy TARGET to the binary directory (needed for linking at
# runtime)
BINTARGET	?= $(DROPS_STDDIR)/bin/$(subst -,/,$(SYSTEM)/$(TARGET))
BINTARGET_LOCAL	?= $(OBJ_BASE)/bin/$(subst -,/,$(SYSTEM)/$(TARGET))
INSTALLFILE_BIN_LOCAL ?= cp $(1) $(2) && chmod 644 $(2)

all:: $(BINTARGET_LOCAL) $(OBJ_BASE)/bin/$(subst -,/,$(SYSTEM)/$(TRG))
install:: $(BINTARGET)

$(BINTARGET) $(BINTARGET_LOCAL): $(TARGET)
	@$(INSTALL_LOCAL_MESSAGE)
	$(if $(INSTALLFILE_BIN_LOCAL),$(VERBOSE)$(INSTALL) -d $(dir $@))
	$(VERBOSE)$(call INSTALLFILE_BIN_LOCAL,$<,$@)

.PHONY: $(BINTARGET)

$(OBJ_BASE)/bin/$(subst -,/,$(SYSTEM)/$(TRG)): os2app.mp
	awk -f $(PKGDIR)/tools/map.awk $< | sort >os2app.mp1
	wc -l os2app.mp1 | awk '{print $$1}' >os2app.mp2
	cat os2app.mp2 os2app.mp1 >$@

endif
