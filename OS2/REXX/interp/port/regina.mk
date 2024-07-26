# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PORT_NAME = os2$(SEP)regina
PORT_TYPE = svn
PORT_URL  = https://svn.code.sf.net/p/regina-rexx/code/interpreter/
PORT_REV  = tags/3.9.6/
PORT_PATCHES  = regina.diff regina2.diff regina3.diff

!include $(%ROOT)tools/mk/port.mk
