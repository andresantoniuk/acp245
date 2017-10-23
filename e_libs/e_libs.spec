%define _distname	 	    %(/bin/rpm -qf /etc/redhat-release | /bin/sed -e 's/-[^-]*$//;s/redhat/rh/;s/fedora/fc/;s/-release//;s/[-.]//g')

Name:		e_libs
Version:	1.1.0
Release:	1%{?_distname}
Summary:	Edantech embedded libraries

Group:		System Environment/Libraries
License:	(c) Edantech
URL:		http://www.edantech.com/sources/e_libs/e_libs.%{version}.tar.gz
Source0:	%{name}-%{version}.tar.gz
BuildRoot:	%(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

Prefix:     /usr

Requires:	    libevent >= 1.4.5

BuildRequires:	make
BuildRequires:	libtool >= 1.5.24
BuildRequires:	autoconf >= 2.61
BuildRequires:	automake >= 1.10
BuildRequires:	gcc >= 4.1.2
BuildRequires:	pkgconfig
BuildRequires:	check-devel >= 0.9.5
BuildRequires:	libevent-devel >= 1.4.5

# docs
BuildRequires:  doxygen >= 1.5.7.1.1
BuildRequires:  texlive-latex

%description
Edantech embedded libraries

%package docs
Summary:    e_libs documentation.
Group:      System Environment/Libraries
License:    (c) Edantech

%description docs
e_libs API reference.

%prep
%setup -q

%build
autoreconf --force --install
%configure --libdir=%{_libdir} --prefix=%{_prefix}
make %{?_smp_mflags}
make check || exit 1
make docs

%install
[ "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf "$RPM_BUILD_ROOT"
make install-strip DESTDIR=$RPM_BUILD_ROOT

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%clean
[ "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf "$RPM_BUILD_ROOT"

%files
%defattr(-,root,root,-)
%doc AUTHORS COPYING NEWS LICENSE

%{_libdir}
%{_includedir}

%files docs
%doc doxygen/html doxygen/latex/refman.pdf

%changelog
* Mon Sep 7 2009 Santiago Aguiar <santiago.aguiar@edantech.com> - 1.0.0
- Added doc package.
* Tue Jun 2 2009 Santiago Aguiar <santiago.aguiar@edantech.com> - 0.1.2
- Released version 0.1.2.
* Fri May 8 2009 Santiago Aguiar <santiago.aguiar@edantech.com> - 0.1.1
- Released version 0.1.1.
* Tue Mar 17 2009 Santiago Aguiar <santiago.aguiar@edantech.com> - 0.1.0
- Released version 0.1.0.
