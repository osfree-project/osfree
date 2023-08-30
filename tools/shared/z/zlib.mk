# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PORT_NAME = tools$(SEP)zlib
PORT_TYPE = wget
PORT_URL  = https://www.zlib.net/zlib13.zip
PORT_REV  = 
PORT_PATCHES  = 

!include $(%ROOT)tools/mk/port.mk
