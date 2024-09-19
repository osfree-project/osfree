# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PORT_NAME = os2$(SEP)libpng
PORT_TYPE = wget
PORT_URL  = https://download.sourceforge.net/libpng/lpng1644.zip
PORT_REV  = 
PORT_PATCHES  = 

!include $(%ROOT)tools/mk/port.mk
