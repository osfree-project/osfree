# This file is an example for defining configurations for different boot
# methods:
#   - QEMU (qemu)
#   - Fiasco-UX (ux)
#   - Image mode (image)
#   - ISO images (grub1iso, grub2iso)
#   - VirtualBox
#   - Fastboot
#
# This is a 'make' file.
#
# Copy this example file to Makeconf.boot in the same directory.
#
# Generic options:
#
# Search path for modules, such as binaries, libraries, kernel, configuration
# files or any other data file you might want to load. Note that the bin and
# lib directories of the build-tree are automatically added to the search
# path.
# MODULE_SEARCH_PATH = /path/to/cfgs:/path/to/foo:..
#
# For additional image building options see the header of
# pkg/bootstrap/server/src/Make.rules

# Add fiasco build directory to MODULE_SEARCH_PATH

MODULE_SEARCH_PATH += $(L4DIR)/conf/examples
MODULE_SEARCH_PATH += $(L4DIR)/pkg/io/io/config
MODULE_SEARCH_PATH += $(L4DIR)/pkg/examples/libs/l4re/c++/clntsrv_ns

# For use with multiple platforms and architectures:

FIASCO_PATH-arm-integrator      = /path/to/fiasco-build-arm-integrator
FIASCO_PATH-arm-rv_vexpress     = /path/to/fiasco-build-arm-rv_vexpress
FIASCO_PATH-arm-rv_vexpress_a15 = /path/to/fiasco-build-arm-rv_vexpress_a15

FIASCO_PATH-mips-malta          = /path/to/fiasco-build-mips-malta

FIASCO_PATH-x86                 = /path/to/fiasco-build-x86
FIASCO_PATH-amd64               = /path/to/fiasco-build-amd64

MODULE_SEARCH_PATH += $(FIASCO_PATH-$(ARCH)-$(PLATFORM_TYPE))
MODULE_SEARCH_PATH += $(FIASCO_PATH-$(ARCH))

# QEMU: Specific configuration for 'qemu' target (make qemu E=xxx'):

# Optional options for QEMU, but setting '-serial stdio' is recommended
QEMU_OPTIONS       ?= -serial stdio
#QEMU_OPTIONS      ?= -nographic

QEMU_OPTIONS-arm-rv              += -M realview-eb -m 256 
#QEMU_OPTIONS-arm-rv              += -cpu arm926
QEMU_OPTIONS-arm-rv              += -cpu arm1176
#QEMU_OPTIONS-arm-rv              += -cpu cortex-a8
#QEMU_OPTIONS-arm-rv              += -cpu cortex-a9
QEMU_OPTIONS-arm-integrator      += -M integratorcp -m 256
#QEMU_OPTIONS-arm-integrator      += -cpu arm1176
QEMU_OPTIONS-arm-rv_vexpress     += -M vexpress-a9 -m 1024 -cpu cortex-a9 -smp 4
QEMU_OPTIONS-arm-rv_vexpress_a15 += -M vexpress-a15 -m 2047 -cpu cortex-a15 \
                                    -smp 4
QEMU_OPTIONS-arm-rv_pbx          += realview-pbx-a9 -m 512 -cpu cortex-a9 -smp 4

QEMU_OPTIONS-mips-malta          += -M malta -m 256 -cpu P5600

QEMU_OPTIONS-arm  += $(QEMU_OPTIONS-arm-$(PLATFORM_TYPE))
QEMU_OPTIONS-mips += $(QEMU_OPTIONS-mips-$(PLATFORM_TYPE))
QEMU_OPTIONS      += $(QEMU_OPTIONS-$(ARCH))

# The path to the QEMU binary - optional
#QEMU_PATH-x86   = /path/to/qemu
#QEMU_PATH-amd64 = /path/to/qemu-system-x86_64
#QEMU_PATH       = $(QEMU_PATH-$(ARCH))


# Fiasco-UX: Specific configuration for 'ux' target ('make ux E=xxx'):
#
# Add Fiasco-UX build directory to MODULE_SEARCH_PATH based
# on the make target

ux: MODULE_SEARCH_PATH = /path/to/fiasco-ux-build:$(MODULE_SEARCH_PATH)

# Additional (optional) variables:
#  UX_OPTIONS: Options for Fiasco-UX, such as -m
#  UX_GFX:     Graphical console resolution and colordepth, e.g.:
#              UX_GFX=800x600@16
#  UX_GFX_CMD: Path to ux_con binary, default is to search in the
#              UX_SEARCH_PATH
#  UX_NET:     Enable network if set, e.g.: UX_NET=y
#  UX_NET_CMD: Path to ux_net binary, default is to search in the
#              UX_SEARCH_PATH 

UX_OPTIONS     = -m 128

# VirtualBox
#
# To use VirtualBox create a VM in VirtualBox and set the name of the VM in
# 'VBOX_VM'. The 'vbox' target uses an ISO image generating target to generate
# an ISO and use that with VirtualBox. 
#
# Required variables:
#  VBOX_VM:        Name of the VM to use.
#
# Additional (optional) variables:
#  VBOX_ISOTARGET: grub1iso or grub2iso (grub2iso is the default)
#  VBOX_OPTIONS:   Additional options (see VBoxSDL --help)
VBOX_VM                = L4
VBOX_ISOTARGET         = grub1iso
#VBOX_OPTIONS          += --memory 256

# Fastboot
# FASTBOOT_BOOT_CMD = path/to/fastboot boot
