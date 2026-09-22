# tools/shared/makefile
#
# Makefile for common shared libs

32_BITS  = 1

DIRS = path ccl toml dep5 git sha1 sha256 json spdx reuse omf lib exe res z

PROJ = tools_shared
TARGET_CLASS = LIBRARY
LIBS = z toml reuse spdx sha1 sha256 json omf lib exe res git dep5 ccl path
PLATFORM = host$(SEP)$(%HOST)$(SEP)

!include $(%ROOT)tools/mk/build.mk

