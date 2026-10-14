# tools/shared/makefile
#
# Makefile for common shared libs

32_BITS  = 1

DIRS = path ccl abi toml dep5 git sha1 sha256 json spdx omf res reuse lib exe dll def map wmp sym z

PROJ = tools_shared
TARGET_CLASS = LIBRARY
LIBS = z toml reuse spdx sha1 sha256 json omf lib exe res git dep5 ccl path abi dll def map wmp sym 
PLATFORM = host$(SEP)$(%HOST)$(SEP)

!include $(%ROOT)tools/mk/build.mk

