%define _distname	 	    %(/bin/rpm -qf /etc/redhat-release | /bin/sed -e 's/-[^-]*$//;s/redhat/rh/;s/fedora/fc/;s/-release//;s/[-.]//g')

Name:		acp245
Version:	1.6.3
Release:	1%{?_distname}
Summary:    Edantech ACP245 protocol message library.

Group:		System Environment/Libraries
License:	(c) Edantech
URL:		http://www.edantech.com/sources/acp245/acp245.%{version}.tar.gz
Source0:	%{name}-%{version}.tar.gz
BuildRoot:	%(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

Prefix:     /usr

Requires:	    e_libs >= 1.1.0
BuildRequires:	libtool >= 1.5.24
BuildRequires:	autoconf >= 2.61
BuildRequires:	automake >= 1.10
BuildRequires:	gcc >= 4.1.2
BuildRequires:	pkgconfig
BuildRequires:	check-devel >= 0.9.5
BuildRequires:	e_libs >= 1.0.0
#BuildRequires:	Cython >= 0.10.3

# python
BuildRequires: python >= 2.5
BuildRequires: python-cheetah
#BuildRequires: Cython >= 0.10.3
BuildRequires: e_libs >= 1.1.0
BuildRequires: python-devel
BuildRequires: synopsis

# docs
BuildRequires:  doxygen >= 1.5.7.1.1
BuildRequires:  texlive-latex

%description
Edantech ACP245 protocol message library.

%package python
Summary: Python bindings for Edantech ACP245 library
Group:   System Environment/Libraries
License: (c) Edantech

Requires: acp245 = %{version}-%{release}
Requires: python >= 2.5
Requires: e_libs >= 1.0.0

%description python
Python bindings for Edantech ACP245 protocol message library.

%package docs
Summary: Edantech ACP245 API reference.
Group:   System Environment/Libraries
License: (c) Edantech

%description docs
ACP245 API reference.

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
make install-strip DESTDIR=$RPM_BUILD_ROOT prefix=%{_prefix} libdir=%{_libdir}

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%clean
[ "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf "$RPM_BUILD_ROOT"

%files
%defattr(-,root,root,-)
%doc AUTHORS COPYING NEWS LICENSE

%{_libdir}/libacp245*
%{_libdir}/pkgconfig*
%{_includedir}/

%files python
/usr/lib*/python*/site-packages/

%files docs
%doc doxygen/html doxygen/latex/refman.pdf

%changelog
* Mon Sep 11 2009 Santiago Aguiar <santiago.aguiar@edantech.com> - 1.1.0
- Updated e_libs dependencies.
* Mon Sep 7 2009 Santiago Aguiar <santiago.aguiar@edantech.com> - 1.0.0
- Added docs package.
* Fri May 8 2009 Santiago Aguiar <santiago.aguiar@edantech.com> - 0.1.3
- Releaded version 0.1.3.
* Tue Mar 17 2009 Santiago Aguiar <santiago.aguiar@edantech.com> - 0.1.0
- Released version 0.1.0.
