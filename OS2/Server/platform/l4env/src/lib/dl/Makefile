PKGDIR		?= ../../..
REP_DIR		?= $(PKGDIR)

include $(REP_DIR)/mk/osfree.mk

TARGET		= libdl.a
MODE		= l4env
SYSTEMS		 = x86-l4v2 arm-l4v2 amd64-l4v2
PRIVATE_INCDIR	 = $(L4DIR)/pkg/ldso/lib/libdl \
		   $(L4DIR)/pkg/ldso/lib/ldso \
		   $(L4DIR)/pkg/ldso/lib/ldso/include \
		   $(L4DIR)/pkg/ldso/lib/ldso/include/sysdeps/linux/$(LDSO_ARCH_NAME) \
		   $(L4DIR)/pkg/ldso/lib/ldso/include/sysdeps/linux/common \
		   $(L4DIR)/pkg/ldso/lib/ldso/include/ldso/$(LDSO_ARCH_NAME) \
		   $(L4DIR)/pkg/ldso/lib/ldso/ARCH-$(ARCH) \
		   $(L4DIR)/pkg/ldso/lib/ldso/include/ldso
SRC_C		= libdl.c # $(REP_DIR)/src/lib/dl/libdl.c # $(L4DIR)/pkg/ldso/lib/libdl/libdl.c
SRC_S           = $(L4DIR)/pkg/ldso/lib/ldso/ARCH-$(ARCH)/resolve.S
LIBS		= -lgeneric_fprov $(ROOTLIB) -ll4util -llist_alloc -l4sys -ldemangle # $(LIBS-y) $(GCCLIB)
LDFLAGS		= 
LDSO_ARCH_NAME_x86   = i386
LDSO_ARCH_NAME_amd64 = x86_64
LDSO_ARCH_NAME_arm   = arm
LDSO_ARCH_NAME	= $(LDSO_ARCH_NAME_$(ARCH))
DEFINES		 = -USHARED -DUCLIBC_RUNTIME_PREFIX=\"/\"
CFLAGS		+= -include $(L4DIR)/pkg/uclibc/lib/contrib/uclibc/include/libc-symbols.h \
		   -DLDSO_ELFINTERP=\"ARCH-$(ARCH)/elfinterp.c\"
OPTS		 = -Os -fno-strict-aliasing
CRT0		 =

# for debugging don't edit this file buf create ./Makeconf.local with DEBUG=1
# be aware that enabling DEBUG here depends on DEBUG in ldso too because of
# mmap_list_regions()
DEBUG		 ?= 0

vpath %.c $(REP_DIR)/src/lib/dl

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
