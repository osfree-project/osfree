#
# A main Makefile for OS/3 boot sequence project
# (c) osFree project.
# valerius, 2006/10/30
#

# TOOLS target comes first because all of them required for build process
# DOS target comes first because many dos executables will be joined with
# some of os/2 executables as dos stubs
#
DIRS = tools bootseq DOS OS2

!include $(%ROOT)/mk/all.mk

#TARGETS = subdirs
#TARGET = all


