# tools/makefile
#
# A Makefile for OS/3 build tools
# (c) osFree project,
# valerius, 2006/10/30
#

# Notes:
# 1. UniAPI must come first here because used to produce API headers

# Note II: Do not list 'scripts' dir here, in this case you'll encounter the dead loop
DIRS = shared sed UNI dos2unix &
       yacc lex jwasm awk &
       mkmsgf msgextrt exehdr bin2c mkctxt genext2fs &
       qemu-img hlldump mapsym renmodul &
       target emxdoc bind spdx-lint spdx-sbom spdx-annotate spdx-merge 

#       lxlite &
# rexxwrap ltools &

PLATFORM = host$(SEP)$(%HOST)$(SEP)

!include $(%ROOT)tools/mk/build.mk
