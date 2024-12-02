# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PORT_NAME = tools$(SEP)lxlite
PORT_TYPE = git
PORT_URL  = https://github.com/bitwiseworks/lxlite
PORT_REV  = master
PORT_PATCHES  = lxlite.diff lxlite2.diff

!include $(%ROOT)tools/mk/port.mk
