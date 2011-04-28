PKGDIR		?= ..
L4DIR		?= $(PKGDIR)/../../../..

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
api/api.c

# if your server implements the server side of an idl defined in an idl-file
# of your package, list the idl file name(s) here (no path needed)
SERVERIDL	= os2fs.idl

# list additional library paths and libraries here
LIBS		= -los2exec -los2fs -lthread -lgeneric_fprov-server # -los2common

#LIBS		= -lgeneric_fprov -lparsecmdline -lloaderif -lthread -lcon -lcontxt -lconstream-server
CFLAGS          = -I$(PKGDIR)/include
MODE=l4env_base
include $(L4DIR)/mk/prog.mk
