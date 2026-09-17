# makefile
#
# A main Makefile for osFree project
# (c) osFree project.
# valerius, 2006/10/30
#

# TOOLS must not be included here because it build by default
# DOS target comes before OS2 target because many DOS executables will be joined with
# some of OS/2 executables as dos stubs.
# SOM target comes before OS2 target because used to build WPS classes

DIRS = msg &
       bootseq &
       DOS &
       OS2 &
       dual #&
#       SOM

!include $(%ROOT)tools/mk/all.mk
