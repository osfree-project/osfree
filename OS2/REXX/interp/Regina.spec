%define debug_package %{nil}
%define vermajor 3
%define verminor 9
Name: Regina-REXX
Version: 3.9.1
Release: 1
Group: Development/Languages/Other
Source: %{name}-%{version}.tar.bz2
BuildRoot: %{_tmppath}/%{name}-%{version}-root
Prefix: /usr
License: LGPL
Vendor: Mark Hessling
URL: http://regina-rexx.sourceforge.net
Summary: Regina Rexx Interpreter binaries, language files and sample programs
%ifarch x86_64 ia64 ppc64 s390x sparc64
# Provides: _{name}(64bit)
#Requires: %{name}-lib(64bit)
%else
# Provides: _{name}
#Requires: %{name}-lib
%endif
# if we don't have _extension define, define it
# this is because Mandriva defines _extension
%{!?_extension: %define _extension .gz}
#PreReq:         %fillup_prereq %insserv_prereq

%description
Regina is an implementation of a Rexx interpreter, compliant with
the ANSI Standard for Rexx (1996).  It is also available on several other
operating systems.

For more information on Regina, visit http://regina-rexx.sourceforge.net/
For more information on Rexx, visit http://www.rexxla.org

%package devel
%ifarch x86_64 ia64 ppc64 s390x sparc64
#Provides: _{name}-dev(64bit) libregina.so()(64bit)
#Requires: %{name}-lib(64bit)
%else
#Provides: _{name}-dev libregina.so()
#Requires: %{name}-lib
%endif
Group: Development/Languages/Other
Summary: Regina Rexx development libraries and header file
%description devel
Regina is an implementation of a Rexx interpreter, compliant with
the ANSI Standard for Rexx (1996).  It is also available on several other
operating systems.

For more information on Regina, visit http://regina-rexx.sourceforge.net/
For more information on Rexx, visit http://www.rexxla.org

%package lib
%ifarch x86_64 ia64 ppc64 s390x sparc64
#Provides: _{name}-lib(64bit)
%else
#Provides: _{name}-lib
%endif
Group: Development/Languages/Other
Summary: Regina Rexx runtime libraries
%description lib
Regina is an implementation of a Rexx interpreter, compliant with
the ANSI Standard for Rexx (1996).  It is also available on several other
operating systems.

For more information on Regina, visit http://regina-rexx.sourceforge.net/
For more information on Rexx, visit http://www.rexxla.org

%prep
%setup

%build
./configure --prefix=%{prefix} --mandir=%{_mandir} --sysconfdir=%{_initrddir} --with-addons-dir=%{_datadir}/%{name}/addons
make

%install
rm -fr %{buildroot}
make DESTDIR=%{buildroot} install

%files
%defattr(-,root,root,-)
%{_mandir}/man1/regina.1%{_extension}
%{_mandir}/man1/rxstack.1%{_extension}
%{_mandir}/man1/rxqueue.1%{_extension}
%{_initrddir}/rxstack
%dir %{_datadir}/%{name}
%{_datadir}/%{name}/examples/rexxcps.rexx
%{_datadir}/%{name}/examples/animal.rexx
%{_datadir}/%{name}/examples/block.rexx
%{_datadir}/%{name}/examples/dateconv.rexx
%{_datadir}/%{name}/examples/timeconv.rexx
%{_datadir}/%{name}/examples/newstr.rexx
%{_datadir}/%{name}/examples/dynfunc.rexx
%{_datadir}/%{name}/examples/regutil.rexx
%{_datadir}/%{name}/de.mtb
%{_datadir}/%{name}/en.mtb
%{_datadir}/%{name}/es.mtb
%{_datadir}/%{name}/no.mtb
%{_datadir}/%{name}/pl.mtb
%{_datadir}/%{name}/pt.mtb
%{_datadir}/%{name}/sv.mtb
%{_datadir}/%{name}/tr.mtb
%{_bindir}/rexx
%{_bindir}/regina
%{_bindir}/rxqueue
%{_bindir}/rxstack
%{_datadir}/%{name}/addons/librxtest1.so
%{_datadir}/%{name}/addons/librxtest2.so

%files devel
%defattr(-,root,root,-)
%{_libdir}/libregina.so
%{_libdir}/libregina.a
%{_includedir}/rexxsaa.h
%{_mandir}/man1/regina-config.1%{_extension}
%{_bindir}/regina-config

%files lib
%defattr(-,root,root,-)
%{_libdir}/libregina.so.%{vermajor}.%{verminor}
%{_libdir}/libregina.so.%{vermajor}
%{_datadir}/%{name}/addons/libregutil.so

#******************************************************************************
%post
# Add the rxstack service
%if 0%{?suse_version}
%fillup_and_insserv rxstack
%endif
ldconfig

#******************************************************************************
%preun
%if 0%{?suse_version}
%stop_on_removal rxstack
%endif

#******************************************************************************
%postun
%if 0%{?suse_version}
%restart_on_update rxstack
%insserv_cleanup
%endif
ldconfig

#******************************************************************************
%post lib
ldconfig

#******************************************************************************
%postun lib
ldconfig

#******************************************************************************
%post devel
ldconfig

#******************************************************************************
%postun devel
ldconfig

#******************************************************************************
%changelog
