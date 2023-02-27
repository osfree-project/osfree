# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PORT_NAME = 4os2
PORT_TYPE = git
PORT_URL  = https://github.com/StevenLevine/4os2
PORT_REV  = main
PORT_PATCHES  = 4os2.diff

!include $(%ROOT)tools/mk/port.mk
