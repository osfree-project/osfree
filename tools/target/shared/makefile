# tools/shared/makefile
#
# Makefile for common shared libs

32_BITS  = 1

DIRS = ccl toml omf exe lib

PROJ = tools_shared
TARGET_CLASS = LIBRARY
LIBS = toml ccl omf exe lib
PLATFORM = host$(SEP)$(%HOST)$(SEP)

!include $(%ROOT)tools/mk/build.mk

