# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PORT_NAME = os2$(SEP)libgd
PORT_TYPE = git
PORT_URL  = https://github.com/libgd/libgd
PORT_REV  = gd-2.3.3
PORT_PATCHES = gdlib.diff

!include $(%ROOT)tools/mk/port.mk
