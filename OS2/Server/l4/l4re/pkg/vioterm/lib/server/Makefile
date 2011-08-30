# directories we need to know
PKGDIR ?= ../..
L4DIR  ?=  $(PKGDIR)/../../../..

SYSTEMS = x86-l4v2 arm-l4v2 amd64-l4v2

# source files
SERVERIDL_libl4vfs_term_server_internal-server.a   = internal.idl
SERVERIDL_libl4vfs_term_server_main-server.a       = main.idl

SRC_C   = 

MODE    = l4env_minimal

TARGET  = libl4vfs_term_server_internal-server.a  \
          libl4vfs_term_server_main-server.a

# include lib role
include $(L4DIR)/mk/lib.mk
