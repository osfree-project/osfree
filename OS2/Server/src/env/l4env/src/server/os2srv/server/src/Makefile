PKGDIR		?= ../..
REP_DIR		?= $(PKGDIR)/../../..

include $(REP_DIR)/mk/osfree.mk

TARGET		= os2srv
# the default relocation address. This may be superseded by a STATIC file.
DEFAULT_RELOC	= 0x01800000

# list your .c files here
#  os2server.c
SRC_C		= \
main.c  \
sysinit.c \
exec.c \
Shared/cfgparser.c \
Shared/processmgr.c \
api/api.c \
api/cfg.c

# Shared/cfgparser.c

# if your server implements the server side of an idl defined in an idl-file
# of your package, list the idl file name(s) here (no path needed)
SERVERIDL	= os2server.idl

# list additional library paths and libraries here
LIBS		= -los2app -los2compat -lgeneric_fprov -los2server -los2fs -lthread -lloaderif -ldm_phys \
		  -lparsecmdline -ll4env_err -levents -lhandle # -los2common 

#LDFLAGS		= --dynamic-linker libkal.s.so \
#		  --export-dynamic \
#		  --rpath-link $(OBJ_BASE)/lib/$(ARCH)_$(CPU)/$(L4API)

#LIBS		= -lgeneric_fprov -lparsecmdline -lloaderif -lthread -lcon -lcontxt -lconstream-server
CFLAGS          += -I$(PKGDIR)/include
MODE=l4env_base
include $(L4DIR)/mk/prog.mk
