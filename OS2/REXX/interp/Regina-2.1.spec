Summary: Regina Rexx Interpreter
Name: Regina
Version: 2.1
Release: 1
Copyright: LGPL
Group: Development/Languages
Source: ftp://ftp.lightlink.com/pub/hessling/Regina/Regina-2.1.tar.gz
%description
Regina is an implementation of a Rexx interpreter, mostly compliant with
the ANSI Standard for Rexx (1996).  It is also available on several other
operating systems. For more information on Rexx, visit http://www.rexxla.org
%prep
%setup

%build
./configure
make

%install
make install

%files
/usr/local/bin/rexx
/usr/local/bin/regina
/usr/local/lib/libregina.a
/usr/local/man/man1/rexx.1
/usr/local/include/rexxsaa.h
/usr/local/lib/libregina.so.2.1
/usr/local/rexx/rexxcps.rexx

%post
ldconfig /usr/local/lib
ln -s /usr/local/lib/libregina.so.2.1 /usr/local/lib/libregina.so.2
ln -s /usr/local/lib/libregina.so.2.1 /usr/local/lib/libregina.so
