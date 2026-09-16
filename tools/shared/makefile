# Makefile for common shared libs

32_BITS  = 1

DIRS = toml reuse spdx sha1 sha256 json omf res git dep5 z

PROJ = tools_shared
TARGET_CLASS = LIBRARY
LIBS = z toml reuse_parser spdx sha1 sha256 json omf res git dep5
PLATFORM = host$(SEP)$(%HOST)$(SEP)

!include $(%ROOT)tools/mk/build.mk

