PKGDIR		?= ../..
REP_DIR		?= $(PKGDIR)/../../..

include $(REP_DIR)/mk/osfree.mk

TARGET		= os2app
TRG             = kal.map
MODE		= l4env_base
SYSTEMS		= x86-l4v2 # arm-l4v2 amd64-l4v2
SRC_C		= main.c dl.c startup.c rpc.c \
                  api.c \
                  kal/kal.c kal/start.c kal/util.c
# NO_DEFAULT_RELOC= 1
# LDFLAGS		= -shared
#NO_DEFAULT_RELOC= 1
#LDFLAGS		= --no-undefined \
#		  --dynamic-linker libld-l4.s.so \
#		  --export-dynamic \
#		  --rpath-link $(OBJ_BASE)/lib/$(ARCH)_$(CPU)/$(L4API)
#LDSCRIPT	= $(call findfile,main_dyn.ld,$(L4LIBDIR))
LIBS		= -los2exec -los2fs -los2server -los2app -los2compat -lloaderif -levents \
                  -ldl -llist_alloc -lgeneric_fprov -ldemangle -ldm_phys
#                  -Bdynamic -lkal.s
DEFAULT_RELOC   = 0xbb000000
SERVERIDL       = os2app.idl
CFLAGS          += -I$(OS3_DIR)/include/os2 -I$(OS3_DIR)/include \
                   -I$(OS3_DIR)/shared/app/os2app/api

vpath %.c $(OS3_DIR)/shared/app/os2app
vpath %.c $(OS3_DIR)/shared/app/os2app/api
vpath %.c $(REP_DIR)/src/server/shared/startup

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

$(OBJ_BASE)/bin/$(subst -,/,$(SYSTEM)/$(TRG)): $(filter os2app, $(TARGET))
	nm $< | grep Kal | awk '{printf "0x%s %s\n", $$1, $$3}' >$<.1
	wc -l $<.1 | awk '{print $$1}' >$@
	cat $<.1   >>$@

endif
