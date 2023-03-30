# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PORT_NAME = find-os2
PORT_TYPE = git
PORT_URL  = https://github.com/FDOS/find
PORT_REV  = master
PORT_PATCHES  = find-os2.diff

!include $(%ROOT)tools/mk/port.mk
