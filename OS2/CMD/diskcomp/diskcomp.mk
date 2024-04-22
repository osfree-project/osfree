# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PORT_NAME = diskcomp
PORT_TYPE = git
PORT_URL  = https://github.com/FDOS/diskcomp/
PORT_REV  = 
PORT_PATCHES  = diskcomp.diff

!include $(%ROOT)tools/mk/port.mk
