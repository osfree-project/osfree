# tools/shared/makefile
#
# Makefile for common shared libs

32_BITS  = 1

DIRS = dlist toml dep5 reuse spdx sha1 sha256 json omf res git z

PROJ = tools_shared
TARGET_CLASS = LIBRARY
LIBS = z toml reuse spdx sha1 sha256 json omf res git dep5 dlist
PLATFORM = host$(SEP)$(%HOST)$(SEP)

!include $(%ROOT)tools/mk/build.mk

