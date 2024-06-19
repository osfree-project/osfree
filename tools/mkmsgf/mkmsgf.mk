# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PORT_NAME    = tools$(SEP)mkmsgf
PORT_TYPE    = git
PORT_URL     = https://github.com/MikeyG/mkmsgf
PORT_REV     = 1.1
PORT_PATCHES = mkmsgf.diff mkmsgf2.diff mkmsgf3.diff mkmsgf4.diff &
    mkmsgf5.diff mkmsgf6.diff mkmsgf7.diff mkmsgf8.diff mkmsgf9.diff
!include $(%ROOT)tools/mk/port.mk
