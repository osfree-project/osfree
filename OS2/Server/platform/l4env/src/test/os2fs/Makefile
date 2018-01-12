PKGDIR		?= ../../..
REP_DIR		?= $(PKGDIR)

include $(REP_DIR)/mk/osfree.mk

TARGET		= os2fs-test
# the default relocation address. This may be superseded by a STATIC file.
DEFAULT_RELOC	= 0x01900000

# list your .c files here
#  os2server.c
SRC_C		= main.c ../shared/skel.c

# list additional library paths and libraries here
LIBS		= -los2fs -los2compat

CFLAGS          += -I$(PKGDIR)/include
MODE=l4env_base

vpath %.c $(OS3_DIR)/shared/test/os2fs

include $(L4DIR)/mk/prog.mk
