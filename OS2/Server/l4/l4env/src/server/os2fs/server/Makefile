PKGDIR		?= ..
L4DIR		?= $(PKGDIR)/../../../../..

TARGET		= os2fs
# the default relocation address. This may be superseded by a STATIC file.
DEFAULT_RELOC	= 0x01880000

# list your .c files here
#  os2server.c
SRC_C		= \
main.c      \
utility.c   \
globals.c   \
mountreg.c  \
api/fprov.c \
api/misc.c  \
api/api.c   \
api/glob/glob.c

# if your server implements the server side of an idl defined in an idl-file
# of your package, list the idl file name(s) here (no path needed)
SERVERIDL	= os2fs.idl

# list additional library paths and libraries here
LIBS  = -lgeneric_fprov-server -levents -los2server -los2compat # -Bdynamic -lkal.s -luc_c

#LDFLAGS		= --dynamic-linker libld-l4.s.so \
#		  --export-dynamic \
#		  --rpath-link $(OBJ_BASE)/lib/$(ARCH)_$(CPU)/$(L4API)

#LDFLAGS		= -shared
CFLAGS          = -I$(PKGDIR)/include -I$(PKGDIR)/server/api/glob
MODE            = l4env_base

include $(L4DIR)/mk/prog.mk

