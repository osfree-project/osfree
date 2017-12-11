PKGDIR		?= ../..
REP_DIR		?= $(PKGDIR)/../../..

include $(REP_DIR)/mk/osfree.mk

TARGET		= execsrv
MODE		= l4env_base
SYSTEMS		= x86-l4v2
SRC_C		= main.c startup.c \
		  modmgr.c ixfmgr.c ixfutil.c
CFLAGS          = -I$(PKGDIR)/include
NO_DEFAULT_RELOC= 1
LDFLAGS		= --no-undefined \
		  --dynamic-linker libld-l4.s.so \
		  --export-dynamic \
		  --rpath-link $(OBJ_BASE)/lib/$(ARCH)_$(CPU)/$(L4API)
LDSCRIPT	= $(call findfile,main_dyn.ld,$(L4LIBDIR))
LIBS		= -Bdynamic -lmemmgr.s -ldl.s -Bstatic \
		 -los2common  -los2compat -los2fs -los2server -ll4sys -ll4sys.p \
                  -ldm_phys -ll4env_err -ll4rm -lloaderif -levents
DEFAULT_RELOC   = 0xba000000
SERVERIDL	= os2exec.idl
CFLAGS          += -I$(OS3_DIR)/include/os2 -I$(OS3_DIR)/include

vpath %.c $(OS3_DIR)/src/server/os2exec
vpath %.c $(REP_DIR)/src/server/shared/startup

include $(L4DIR)/mk/prog.mk

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
