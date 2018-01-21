PKGDIR	?= ..
REP_DIR ?= $(PKGDIR)

include $(REP_DIR)/mk/osfree.mk

# the default is to build the listed directories, provided that they
# contain a Makefile. If you need to change this, uncomment the following
# line and adapt it.

PATCHES = l4rm l4vfs modes modules-max

apply:
	@cd $(ROOT) && for file in $(PATCHES); do \
	    patch -p0 <$(OS3_DIR)/platform/l4env/patches/$$file.patch; \
	done

revert:
	@cd $(ROOT) && for file in $(PATCHES); do \
	    patch -p0 -R <$(OS3_DIR)/platform/l4env/patches/$$file.patch; \
	done
