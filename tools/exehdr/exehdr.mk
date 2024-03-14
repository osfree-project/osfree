# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PORT_NAME    = tools$(SEP)exehdr
PORT_TYPE    = git
PORT_URL     = https://greenenet.ddns.net/os2project/exehdr
#PORT_REV     = 1.1
PORT_PATCHES = exehdr.diff
!include $(%ROOT)tools/mk/port.mk
