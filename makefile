# SPDX-FileCopyrightText: (C) 2026 osFree Project
# SPDX-License-Identifier: BSD-3-Clause
# makefile
#
# A main Makefile for osFree project
# (c) osFree project.
# valerius, 2006/10/30
#

# TOOLS must not be included here because it build by default
# msg used by most of personalities, so ges first.
# DOS target comes before OS2 target because many DOS executables will be joined with
# some of OS/2 executables as dos stubs.
# @todo SOM depends on OS/2 build dir!!! SOM target comes before OS2 target because used to build WPS classes

DIRS = msg &
       bootseq &
       DOS &
       OS2 &
       dual &
       SOM &
       PKGS

!include $(%ROOT)tools/mk/all.mk
