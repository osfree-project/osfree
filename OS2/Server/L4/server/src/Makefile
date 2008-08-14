PKGDIR		?= ../..
L4DIR		?= $(PKGDIR)/../..

TARGET		= os2server
# the default relocation address. This may be superseded by a STATIC file.
DEFAULT_RELOC	= 0x01800000

# list your .c files here
SRC_C		= os2server.c

# if your server implements the server side of an idl defined in an idl-file
# of your package, list the idl file name(s) here (no path needed)
SERVERIDL	= os2server.idl

# list additional library paths and libraries here
LIBS		= -lgeneric_fprov -lparsecmdline -lloaderif -lthread -lcon -lcontxt -lconstream-server

include $(L4DIR)/mk/prog.mk
