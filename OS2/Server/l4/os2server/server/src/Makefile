PKGDIR		?= ../..
L4DIR		?= $(PKGDIR)/../..

TARGET		= os2server
# the default relocation address. This may be superseded by a STATIC file.
DEFAULT_RELOC	= 0x01800000

# list your .c files here
#  os2server.c
SRC_C		= main.c utility.c io.c MountReg.c globals.c native_dynlink.c l4_alloc_mem.c \
Shared/token.c \
Shared/modmgr.c \
Shared/ixfmgr.c \
Shared/ixfmgr_lx.c \
Shared/ixfmgr_lx_load.c \
Shared/ixfmgr_ne.c \
Shared/ixfmgr_lx_fixuplx.c \
Shared/ixfmgr_lx_loadobjlx.c \
Shared/ixfmgr_lx_modlx.c \
Shared/ixfmgr_lx_execlx.c \
Shared/ixfmgr_lx_debuglx.c \
Shared/memmgr.c \
Shared/api/apistub.c \
Shared/api/api.c \
Shared/cfgparser.c \
Shared/processmgr.c \
#Shared/startup.c

#Shared/ow_dlfcn.c \

# if your server implements the server side of an idl defined in an idl-file
# of your package, list the idl file name(s) here (no path needed)
SERVERIDL	= os2server.idl

# list additional library paths and libraries here
LIBS		=  -los2server -lparsecmdline -lthread # -lcon -lcontxt -lconstream-server
#LIBS		= -lgeneric_fprov -lparsecmdline -lloaderif -lthread -lcon -lcontxt -lconstream-server
CFLAGS          = -I$(PKGDIR)/include
MODE=l4env_base
include $(L4DIR)/mk/prog.mk
