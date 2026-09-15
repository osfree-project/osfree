# Makefile for common shared libs

32_BITS  = 1

PROJ = tools_shared
TARGET_CLASS = LIBRARY
DIRS = z reuse spdx sha1 sha256 json omf res git dep5
LIBS = z reuse_parser spdx sha1 sha256 json omf res git dep5
PLATFORM = host$(SEP)$(%HOST)$(SEP)

!include $(%ROOT)tools/mk/build.mk

