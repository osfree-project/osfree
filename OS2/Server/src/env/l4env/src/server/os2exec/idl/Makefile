# 
# makefile for package IDL directory
#

PKGDIR  ?= ..
REP_DIR ?= $(PKGDIR)/../../..

include $(REP_DIR)/mk/osfree.mk

# create an idl file an assign its name to IDL
IDL	= os2exec.idl

IDL_FLAGS = -t $(cdefs)

# the default idl type is DCE/dice. Uncomment the next line to change this.
# IDL_TYPE= <your choice>

include $(L4DIR)/mk/idl.mk
