Summary: Regina Rexx Interpreter
Name: Regina
Version: 3.3
Release: 1
Copyright: 1992-1994 Anders Christensen, 1994-2003 Mark Hessling
Group: Development/Languages
Source: ftp://prdownloads.sourceforge.net/regina-rexx/Regina-REXX-3.3.tar.gz
Distribution: LGPL
Vendor: Mark Hessling
Packager: Mark Hessling
URL: http://regina-rexx.sourceforge.net
Provides: libregina.so Regina-REXX

%description
Regina is an implementation of a Rexx interpreter, compliant with
the ANSI Standard for Rexx (1996).  It is also available on several other
operating systems. 
For more information on Regina, visit http://regina-rexx.sourceforge.net/
For more information on Rexx, visit http://www.rexxla.org
%prep
%setup

%build
./configure
make

%install
make install

%files
/usr/bin/rexx
/usr/bin/regina
/usr/bin/rxqueue
/usr/bin/rxstack
/usr/lib/libregina.a
/usr/man/man1/regina.1
/usr/include/rexxsaa.h
/usr/lib/libregina.so.3.3
/usr/share/regina/rexxcps.rexx
/usr/etc/rc.d/init.d/rxstack

%post
cd /usr/lib
ln -sf ./libregina.so.3.3 ./libregina.so.3
ln -sf ./libregina.so.3.3 ./libregina.so
ldconfig /usr/lib
