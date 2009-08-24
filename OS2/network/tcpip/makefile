# $Id: makefile,v 1.1 2004/08/16 06:26:36 prokushev Exp $

ROOT=..\..\..\..
!include $(ROOT)\makefile.inc

# Add other targets here upon their maturity
UTILS=ping # tracerte finger inetd syslogd

all
 @for %%i in ($(UTILS)) do @cd %%i&$(MAKE) $(MAKEFLAGS)&cd ..

clean
 @for %%i in ($(UTILS)) do @cd %%i&$(MAKE) $(MAKEFLAGS) clean&cd ..
