# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PORT_NAME = regina
PORT_TYPE = svn
PORT_URL  = https://svn.code.sf.net/p/regina-rexx/code/interpreter/
PORT_REV  = tags/3.9.5/
PORT_PATCHES  = regina.diff

!include $(%ROOT)tools/mk/port.mk
