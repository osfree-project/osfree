Summary: Regina Rexx Interpreter
Name: Regina
%define vermajor 3
%define verminor 4
Version: 3.4
Release: 1
Group: Development/Languages
Source: Regina-REXX-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-root
Prefix: /usr
License: LGPL
Vendor: Mark Hessling
Packager: Mark Hessling
URL: http://regina-rexx.sourceforge.net
Provides: Regina-REXX %{name}
Icon: regina64.xpm

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
rm -fr %{buildroot}
make DESTDIR=%{buildroot} install

%files
/usr/bin/rexx
/usr/bin/regina
/usr/bin/regina-config
/usr/bin/rxqueue
/usr/bin/rxstack
/usr/%{_lib}/libregina.a
/usr/%{_lib}/libregina.so.%{version}
/usr/%{_lib}/libregina.so.%{vermajor}
/usr/%{_lib}/libregina.so
/usr/%{_lib}/librxtest1.so
/usr/%{_lib}/librxtest2.so
/usr/include/rexxsaa.h
/usr/share/man/man1/regina.1.gz
/usr/etc/rc.d/init.d/rxstack
/usr/share/regina/rexxcps.rexx
/usr/share/regina/animal.rexx
/usr/share/regina/block.rexx
/usr/share/regina/dateconv.rexx
/usr/share/regina/timeconv.rexx
/usr/share/regina/newstr.rexx
/usr/share/regina/dynfunc.rexx
/usr/share/regina/de.mtb
/usr/share/regina/en.mtb
/usr/share/regina/es.mtb
/usr/share/regina/no.mtb
/usr/share/regina/pl.mtb
/usr/share/regina/pt.mtb
/usr/share/regina/tr.mtb

%post
cd %{prefix}/%{_lib}
ln -sf ./libregina.so.%{version} ./libregina.so.%{vermajor}
ln -sf ./libregina.so.%{version} ./libregina.so
ldconfig %{prefix}/%{_lib}
