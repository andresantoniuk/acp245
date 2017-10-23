%define _version            1.8.2
%define _distname	 	    %(/bin/rpm -qf /etc/redhat-release | /bin/sed -e 's/-[^-]*$//;s/redhat/rh/;s/fedora/fc/;s/-release//;s/[-.]//g')
%define _python_version     python%(python -c 'import sys ; print sys.version[:3]')
%define _initdir            /etc/init.d/
%define _confdir            /etc/%{name}/
%define _syscfgdir          /etc/sysconfig/
%define _sbindir            /usr/sbin/
%define _logdir             /var/log/%{name}/
%define _piddir             /var/run/%{name}/
%define _datadir            /var/%{name}/

Name:		acpyd
Version:	%{_version}
Release:	1.%{_distname}
Summary:	Edantech ACP 245 server.
License:	(c) Edantech
Group:		System Environment/Daemons
Source:		%{name}-%{version}.tar.gz
Url:		http://www.edantech.com/sources/acpyd/acpyd.%{version}.tar.gz
Vendor:		Edantech
BuildRoot:	/tmp/%{name}-root
BuildRequires: acp245-python >= 1.6.0
BuildRequires: make
BuildRequires: python >= 2.5
BuildRequires: python-axiom >= 0.5.31
BuildRequires: python-cheetah
BuildRequires: python-epsilon >= 0.5.12
BuildRequires: python-nevow >= 0.9.29
BuildRequires: python-reportlab
BuildRequires: python-setuptools
BuildRequires: python-simplejson
BuildRequires: python-twisted-core >= 2.5.0
Requires: acp245-python >= 1.6.0
Requires: python >= 2.5
Requires: python-axiom >= 0.5.31
Requires: python-epsilon >= 0.5.12
Requires: python-nevow >= 0.9.29
Requires: python-reportlab
Requires: python-setuptools
Requires: python-simplejson
Requires: python-twisted-core >= 2.5.0

Requires(pre): /usr/sbin/useradd

%description
Edantech ACP245 Server.

%pre
# Add the "acpyd" user and group
/usr/sbin/useradd --comment 'acpyd' --shell /sbin/nologin --home '%{_datadir}' -r acpyd 2> /dev/null || :

%post
/sbin/chkconfig --add acpyd
/sbin/chkconfig acpyd resetpriorities
# configure it to start by default
/sbin/chkconfig acpyd on

%preun
if [ $1 = 0 ]; then
    /sbin/chkconfig --del acpyd
fi
# Force stopping of service
%{_initdir}/acpyd stop

%prep
%setup

%build
make
make test || exit 1

%install
rm -rf $RPM_BUILD_ROOT
make install-strip INSTALLOPTS="" DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr (-, root, root)
%doc NEWS COPYING LICENSE examples/ doc/gencert.sh

/usr/lib*/%{_python_version}/
%attr(0755, root, root) /etc/init.d/acpyd
%attr(0750, acpyd, acpyd) %dir %{_logdir}
%attr(0750, acpyd, acpyd) %dir %{_confdir}
%attr(0640, acpyd, acpyd) %config(noreplace) %{_confdir}/acpyd.conf
%attr(0750, acpyd, acpyd) %dir %{_piddir}
%attr(0640, root, root)   %config(noreplace) %{_syscfgdir}/acpyd

%attr(0750, root, root)   %{_sbindir}/acpyd_respawn

%attr(0750, root, acpyd) %dir %{_datadir}
%attr(0440, root, acpyd) %{_datadir}/server.pem
%attr(0750, root, acpyd) %dir %{_datadir}/static/
%attr(0750, root, acpyd) %dir %{_datadir}/static/css
%attr(0640, root, acpyd) %{_datadir}/static/css/*.css
%attr(0750, root, acpyd) %dir %{_datadir}/static/css/images
%attr(0640, root, acpyd) %{_datadir}/static/css/images/*
%attr(0750, root, acpyd) %dir %{_datadir}/static/js
%attr(0640, root, acpyd) %{_datadir}/static/js/*
%attr(0750, root, acpyd) %dir %{_datadir}/templates/
%attr(0640, root, acpyd) %{_datadir}/templates/*
%attr(0640, root, acpyd) %{_datadir}/js/*

%attr(0750, acpyd, acpyd) %dir %{_datadir}/vhosts/

%attr(0750, root, acpyd) %dir %{_datadir}/vhost-skel/
%attr(0750, root, acpyd) %dir %{_datadir}/vhost-skel/console_scripts
%attr(0640, root, acpyd) %{_datadir}/vhost-skel/console_scripts/*

%attr(0750, root, acpyd) %dir %{_datadir}/std_console_scripts
%attr(0640, root, acpyd) %{_datadir}/std_console_scripts/*

# gateway scripts are comon and not writeable
%attr(0750, root, acpyd) %dir %{_datadir}/gateway_scripts
%attr(0750, root, acpyd) %dir %{_datadir}/gateway_scripts/server
%attr(0640, root, acpyd) %{_datadir}/gateway_scripts/server/*
%attr(0750, root, acpyd) %dir %{_datadir}/gateway_scripts/client
%attr(0640, root, acpyd) %{_datadir}/gateway_scripts/client/*

%changelog
* Fri Jul 31 2009 Santiago Aguiar <santiago.aguiar@edantech.com> - 0.2.0
- Updated spec file for vhost support.
* Tue May 19 2009 Santiago Aguiar <santiago.aguiar@edantech.com> - 0.0.5
- Released version 0.0.5.
* Fri May 8 2009 Santiago Aguiar <santiago.aguiar@edantech.com> - 0.0.3
- Released version 0.0.3.
