# tools/makefile
#
# A Makefile for OS/3 build tools
# (c) osFree project,
# valerius, 2006/10/30
#

# Notes:
# 1. UniAPI must come first here because used to produce API headers
# 2. Do not list 'scripts' dir here, in this case you'll encounter the dead loop
# 3. doxy-lint comes first to start doxygen checks as fast as possible
# 4. dos2unix comes second because needed for PORTS

DIRS = doxy-lint dos2unix shared sed UNI &
       yacc lex jwasm awk &
       mkmsgf msgextrt exehdr bin2c mkctxt genext2fs &
       qemu-img hlldump mapsym renmodul &
       target emxdoc bind reuse-lint spdx-sbom reuse-annotate spdx-merge 

#       lxlite &
# rexxwrap ltools &

PLATFORM = host$(SEP)$(%HOST)$(SEP)

!include $(%ROOT)tools/mk/build.mk
