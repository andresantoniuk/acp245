Dependencies
*******************************************************************************
Check acpyd.spec file BuildRequires to see what packages are required to build
and execute acpyd. Install required packages with yum.

To make a release for Windows, you should point the ACPYD_WIN_DEPS environment
variable to a location that has the Windows dependencies for acpyd.
For example:
ACPYD_WIN_DEPS=/mnt/edantech/relases/acpyd-win-deps mkrelease acpyd 1.x.x

ATTENTION
*******************************************************************************
If you the following error:

exceptions.IOError: [Errno 13] Permission denied: '/usr/lib/python2.6/site-packages/twisted/plugins/dropin.cache.new'

It means that the RPM is not yet packaged correctly (Fedora 11 still has this
issue). To avoid that message run 'twistd --help' as root at least once and
the error will stop appearing.

You will get the same error with nevow. Change the permissions of the directory
to allow the creation of the cache file:

chmod 777 /usr/lib/python2.6/site-packages/nevow/plugins

This is a huge security risks, but at the time I don't know (nor researched)
a better way to do it.

Also, both packaged versions of twisted and nevow print ugly warnings
regarding usage of deprecated python API when using Python 2.6.
Ignore them until someone packs a newer version of twisted/nevow.
