PKGDIR		?= ../../..
L4DIR		=  $(PKGDIR)/../..

TARGET		= libkal.s.so
MODE		= l4env_base
SYSTEMS		= x86-l4v2 # arm-l4v2 amd64-l4v2
SRC_C		= api.c start.c wrap.c util.c
LIBS		= -los2exec -los2fs -los2server -los2compat -lloaderif.p -levents  -llist_alloc.p \
		  -lgeneric_fprov.p -ldemangle.p -ldm_mem.p -ldm_generic.p    -lsemaphore.p \
		  -lslab.p -llogserver_capsule.p     -ldm_generic.p -lnames.p \
		  -lsigma0.p -lroot.p -nostdlib -u printf -lc_be_io.o -u mmap_anon -lc_be_simple_mem.p \
		  -lc_be_time -lrtc.p -ldm_mem.p -ldm_generic.p     -lsemaphore.p -ll4env_err.p \
		  -lslab.p -llogserver_capsule.p     -ldm_generic.p -lnames.p -ll4util_root.p -ll4util.p \
		  -lsigma0.p -luc_c  $(GCCLIB)  -u mmap_anon \
		  -lc_be_simple_mem.p -ldm_mem.p -ldm_generic.p -lc_be_time -lc_be_mmap.p -lc_be_mmap_util.p \
		  -lc_be_l4env_start_stop.p -lgeneric_ts.p -lc_be_syslog -lc_be_file-table -luc_be_simple_sleep \
		  -ll4vfs_common_io -ll4vfs_basic_io -ll4vfs_connection -ll4vfs_basic_name_server -ll4vfs_name_server \
		  -ll4vfs_name_space_provider -ll4vfs_extendable -ll4sys.p -luc_c  -ll4util_root.p -ll4util.p -lrtc.p \
		  -lhandle
CFLAGS          = -I$(PKGDIR)/include

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
