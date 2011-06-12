PKGDIR ?= ../..
L4DIR  ?= $(PKGDIR)/../../../..

SYSTEMS = x86-l4v2 arm-l4v2 amd64-l4v2

# the name of your library
TARGET  = libl4vfs_term_server_internal.a \
          libl4vfs_term_server_main.a

# list your .c files here
SRC_C   =

MODE    = l4env_minimal

CLIENTIDL_libl4vfs_term_server_internal.a = internal.idl
CLIENTIDL_libl4vfs_term_server_main.a     = main.idl

include $(L4DIR)/mk/lib.mk
