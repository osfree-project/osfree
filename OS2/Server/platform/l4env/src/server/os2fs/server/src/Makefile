PKGDIR		?= ../..
REP_DIR		?= $(PKGDIR)/../../..

include $(REP_DIR)/mk/osfree.mk

TARGET		= os2fs
# the default relocation address. This may be superseded by a STATIC file.
DEFAULT_RELOC	= 0x01880000

# list your .c files here
#  os2server.c
SRC_C		= \
main.c      \
api/rpc.c   \
api/glob/glob.c \
utility.c   \
globals.c   \
mountreg.c  \
api/api.c   \
api/misc.c

# if your server implements the server side of an idl defined in an idl-file
# of your package, list the idl file name(s) here (no path needed)
SERVERIDL	= os2fs.idl

# list additional library paths and libraries here
LIBS  = -lgeneric_fprov-server -levents -los2server \
        -los2compat -lgeneric_fprov # -ll4util_root -lrtc # -Bdynamic -lkal.s -luc_c

#LDFLAGS		= --dynamic-linker libld-l4.s.so \
#		  --export-dynamic \
#		  --rpath-link $(OBJ_BASE)/lib/$(ARCH)_$(CPU)/$(L4API)

#LDFLAGS		= -shared
CFLAGS          += -I$(PKGDIR)/include -I$(PKGDIR)/server/src/api/glob \
                   -I$(OS3_DIR)/shared/server/os2fs/api
MODE            = l4env_base

vpath %.c $(OS3_DIR)/shared/server/os2fs

include $(L4DIR)/mk/prog.mk
