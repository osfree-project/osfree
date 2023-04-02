# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PORT_NAME = os2$(SEP)openjpeg
PORT_TYPE = git
PORT_URL  = https://github.com/uclouvain/openjpeg
PORT_REV  = v2.5.0

!include $(%ROOT)tools/mk/port.mk
