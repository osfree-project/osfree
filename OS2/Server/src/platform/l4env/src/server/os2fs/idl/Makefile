PKGDIR	?= ..
REP_DIR ?= $(PKGDIR)/../../..

include $(REP_DIR)/mk/osfree.mk

# the default is to build the listed directories, provided that they
# contain a Makefile. If you need to change this, uncomment the following
# line and adapt it.
# TARGET = idl src lib server examples doc
# create an idl file an assign its name to IDL
IDL	= os2fs.idl

IDL_FLAGS = -t $(cdefs)

include $(L4DIR)/mk/idl.mk
