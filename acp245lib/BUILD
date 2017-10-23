** Portability
-------------------------------------------------------------------------------
The code tries to be portable to other unix systems.
By default, is compiled with -std=c89 which disables varidic macros used to
display the line and source file on log messages, but they can be renabled
by modifying configure.ac accordingly.

Each supported platform has it's own folder inside the src/ directory in which
it must include at least the acp245_config.h file with options enabled according
to the platform.

-------------------------------------------------------------------------------
* BUILDING FOR WAVECOM *
-------------------------------------------------------------------------------
A special makefile is provided to use Wavecom build process and toolchain.

To build the project run:

make -f Makefile.wm

To install ACP Plugin in OpenAT directory:

make -f Makefile.wm install

** Dependencies
-------------------------------------------------------------------------------
You need to have installed the gcc arm cross compiler in /usr/local and the
OpenAT SDK in your home directory. This paths are configured by the
OAT_HOME and WM_OAT_IDE_GTS_GCC parameters on the the first lines of
Makefile.wm and can be modified if needed.

To install the Plugin in the OpenAT SDK directory, you need to have permissions
on the Plugin folder (normall $OAT_HOME/Plug-ins).

Please, internally, use a Edantech's OpenAT SDK package, in which permissions
and some minimum updates have been performed to make it work under Linux.
Otherwise, you will need to manually update the OpenAT SDK installation by
following the instructions at the Edantech's Intranet Wiki.

-------------------------------------------------------------------------------
* BUILDING FOR WINDOWS *
-------------------------------------------------------------------------------

** COMPILATION WITH WINE (Recommended)
-------------------------------------------------------------------------------

Follow the instructions to setup a compilation environment using wine and
Visual Studio 2008 as instructed on Edantech wiki.

You will need a working installation of:
* wine
* Visual Studio C++ 2008 installed under wine (copy files from a windows
  install). You can use the Express edition, that you can download freely
  from MSDN.
* NSIS (nullsoft scriptable install system) installed under wine.
* e_libs (using the installer provided by Edantech).
* Python 2.5, to compile the Python Bindings. Cython is not necessary, since we
  use Cython from Linux to generate the .c files and then we compile them with
  Visual Studio C++. Python 2.6 *might* also work, but is not yet tested.

Make sure you install all the tools under Program Files, otherwise you will
need to tweak the Makefile.win.in files.

** COMPILATION WITH MING32 (Not recommended)
-------------------------------------------------------------------------------

You must first build and install e_libs compiled for Windows on your system.

After that, we assume you have all the needed tools and environment settings
to compile acp245 library for windows.

Run:

./autogen.sh --host=i686-pc-mingw32 --prefix=/usr/i686-pc-mingw32/sys-root/mingw/
make
sudo make install
sudo cp src/.libs/*.dll /usr/i686-pc-mingw32/sys-root/mingw/lib/

MING32 compilation has been minimally tested. Also, compilation of Python
bindings using MING32 has not been tested at all.

-------------------------------------------------------------------------------
* BUILDING FOR OTHER TARGETS (Linux and others) *
-------------------------------------------------------------------------------

** Dependencies
-------------------------------------------------------------------------------
Running autogen.sh will check for the required dependencies.

Building tested with:

libtool-1.5.26
autoconf-2.63
automake-1.10.1
gcc-4.3.2

For testing:

check-devel-0.9.6-1.fc10.x86_64
splint-3.1.2-2.fc10.x86_64
valgrind-3.3.0-4.x86_64
gdb-6.8-29.fc10.x86_64
lcov-1.6-1.fc10.noarch

For Fedora Core 10:
yum install gcc libtool autoconf automake check-devel splint valgrind gdb lcov

For building documentation:
yum install doxygen texlive-latex

** Python Bindings
-------------------------------------------------------------------------------

The Python Bindings are written using Cython. You need to have Cython installed
to build them and run the tests, otherwise the 'make test' command will fail.

You will also need to install Synopsis C parser framework, since most of the
Cython code is generated directly from the C header files.

For Fedora Core 10:
yum install Cython synopsis

You need *at least* version 0.10.3 of Cython. If you are using fc9 or earlier,
or CentOS 5, you can rebuild the fc10 src.rpm and install it without problems.

** Source and debug info packages.
-------------------------------------------------------------------------------
It's recommended to install the source and debuginfo for the following 
packages:

glibc
openssl
gdbm

For doing so, you can use yum-utils, and follow the next steps:

# (audit-libs-devel required for patching of glibc sources)
yum install yum-utils rpmbuild audit-libs-devel 

# Install debuginfo packages (debuginfo-install included with yum-utils)
debuginfo-install glibc openssl gdbm

# Download source RPM, (yumdownloader included with yum-utils)
yumdownloader -source glibc openssl gdbm

# Install source RPM, ignore user/group errors (WARN: USING * )
rpm -ivh glibc-*.src.rpm openssl-*.src.rpm gdbm-*.src.rpm 

# Patch installed sources
# Replace /usr/src/redhat/ with user defined RPM build dir if necessary 
# (as defined in ~/.rpmmacros)
rpmbuild -bp /usr/src/redhat/SPECS/glibc.spec 
rpmbuild -bp /usr/src/redhat/SPECS/openssl.spec
rpmbuild -bp /usr/src/redhat/SPECS/gdbm.spec
