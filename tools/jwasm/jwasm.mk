# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PORT_NAME = tools$(SEP)jwasm
PORT_TYPE = git
PORT_URL  = https://github.com/Baron-von-Riedesel/JWasm
PORT_REV  = v2.17

!include $(%ROOT)tools/mk/port.mk
