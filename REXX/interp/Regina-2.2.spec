Summary: Regina Rexx Interpreter
Name: Regina
Version: 2.2
Release: 5
Copyright: LGPL
Group: Development/Languages
Source: ftp://ftp.lightlink.com/pub/hessling/Regina/Regina-REXX-2.2.tar.gz
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
/usr/bin/rexx
/usr/bin/regina
/usr/bin/rxqueue
/usr/bin/rxstack
/usr/lib/libregina.a
/usr/man/man1/rexx.1
/usr/include/rexxsaa.h
/usr/lib/libregina.so.2.2
/usr/share/regina/rexxcps.rexx
/etc/rc.d/init.d/rxstack

%post
cd /usr/lib
ln -sf ./libregina.so.2.2 ./libregina.so.2
ln -sf ./libregina.so.2.2 ./libregina.so
ldconfig /usr/lib
cd /etc/rc.d/rc0.d
ln -sf ../init.d/rxstack ./K10rxstack
cd /etc/rc.d/rc2.d
ln -sf ../init.d/rxstack ./K10rxstack
cd /etc/rc.d/rc6.d
ln -sf ../init.d/rxstack ./K10rxstack
cd /etc/rc.d/rc3.d
ln -sf ../init.d/rxstack ./S99rxstack
cd /etc/rc.d/rc5.d
ln -sf ../init.d/rxstack ./S99rxstack
