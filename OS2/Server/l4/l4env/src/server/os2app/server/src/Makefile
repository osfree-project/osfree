PKGDIR		?= ../..
L4DIR		?= $(PKGDIR)/../../../../..

TARGET		= os2app
TRG             = dl.map
MODE		= l4env_base
SYSTEMS		= x86-l4v2 # arm-l4v2 amd64-l4v2
SRC_C		= main.c $(PKGDIR)/../shared/startup/startup.c dl.c api/api.c
CFLAGS          = -I$(PKGDIR)/include -I$(PKGDIR)/../../../include
# NO_DEFAULT_RELOC= 1
LIBS		= -los2exec -los2fs -los2server -los2compat -lloaderif -levents \
                  -ldl -llist_alloc -lgeneric_fprov -ldemangle
#                  -Bdynamic -lkal.s
# LDFLAGS		= -shared
DEFAULT_RELOC   = 0xbb000000

SERVERIDL       = os2app.idl

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

#$(BINTARGET) $(BINTARGET_LOCAL): $(TARGET)
#	@$(INSTALL_LOCAL_MESSAGE)
#	$(if $(INSTALLFILE_BIN_LOCAL),$(VERBOSE)$(INSTALL) -d $(dir $@))
#	$(VERBOSE)$(call INSTALLFILE_BIN_LOCAL,$<,$@)

.PHONY: $(BINTARGET)

$(OBJ_BASE)/bin/$(subst -,/,$(SYSTEM)/$(TRG)): $(filter os2app, $(TARGET))
	nm $< | grep Dl | awk '{printf "0x%s %s\n", $$1, $$3}' >$<.1
	wc -l $<.1 | awk '{print $$1}' >$@
	cat $<.1   >>$@

endif
