# tools/shared/makefile
#
# Makefile for common shared libs

32_BITS  = 1

DIRS = path ccl toml dep5 reuse spdx sha1 sha256 json omf res git z

PROJ = tools_shared
TARGET_CLASS = LIBRARY
LIBS = z toml reuse spdx sha1 sha256 json omf res git dep5 ccl path
PLATFORM = host$(SEP)$(%HOST)$(SEP)

!include $(%ROOT)tools/mk/build.mk

