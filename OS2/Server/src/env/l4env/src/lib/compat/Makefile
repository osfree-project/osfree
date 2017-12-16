PKGDIR		?= ../../..
REP_DIR		?= $(PKGDIR)

include $(REP_DIR)/mk/osfree.mk

TARGET		= libos2compat.a
MODE		= l4env
SYSTEMS		= x86-l4v2 # arm-l4v2 amd64-l4v2
SRC_C		= log.c fileprov.c thread.c dataspace.c rm.c err.c segment.c \
		  cfgparser.c env.c path.c string.c token.c
LIBS		= 
CFLAGS          += -I$(PKGDIR)/include

vpath %.c = $(OS3_DIR)/src/lib/compat

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
