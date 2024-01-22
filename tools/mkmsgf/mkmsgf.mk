# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PORT_NAME    = tools$(SEP)mkmsgf
PORT_TYPE    = git
PORT_URL     = https://github.com/MikeyG/mkmsgf
PORT_REV     = 1.1
PORT_PATCHES = mkmsgf.diff
!include $(%ROOT)tools/mk/port.mk
