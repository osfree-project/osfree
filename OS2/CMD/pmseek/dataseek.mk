# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PORT_NAME = os2$(SEP)dataseek
PORT_TYPE = svn
PORT_URL  = http://svn.netlabs.org/repos/dataseeker/trunk/
PORT_REV  = 
PORT_PATCHES  = #4os2.diff

!include $(%ROOT)tools/mk/port.mk
