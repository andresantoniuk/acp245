Portability
-------------------------------------------------------------------------------
Code *tries* to be portable to other unix systems. 
By default, is compiled with -std=c89 which disables varidic macros used to
display the line and source file on log messages, but they can be renabled
by modifying configure.ac accordingly.

Each supported platform has it's own folder inside the src/ directory in which
it must include at least the e_port.h, e_mem.c and e_client.c.

-------------------------------------------------------------------------------
* BUILDING FOR WAVECOM *
-------------------------------------------------------------------------------
A special makefile is provided to use Wavecom build process and toolchain.

To build the project run:

make -f Makefile.wm

To install ACP Plugin in OpenAT directory:

make -f Makefile.wm install

You can also run wavecom targets by running:

make wm_<target_name>
ie: make wm_install

Dependencies
-------------------------------------------------------------------------------
You need to have installed the gcc arm cross compiler in /usr/local and the 
OpenAT SDK in your home directory. This paths are configured by the 
OAT_HOME and WM_OAT_IDE_GTS_GCC parameters on the the first lines of
Makefile.wm and can be modified if needed.

To install the Plugin in the OpenAT SDK directory, you need to have permissions
on the Plugin folder (normall $OAT_HOME/Plug-ins).

-------------------------------------------------------------------------------
* BUILDING FOR WINDOWS *
-------------------------------------------------------------------------------

** COMPILATION WITH WINE (Recommended)
-------------------------------------------------------------------------------

Follow the instructions to setup a compilation environment using wine and
Visual Studio 2008 as instructed on Edantech wiki.

You will need a working installation of:
* wine
* Visual Studio C++ 2008 installed under wine (copy files from a windows install)
* NSIS (nullsoft scriptable install system) installed under wine.

Make sure you install all the tools under Program Files.

** COMPILATION WITH MING32 (Not recommended)
-------------------------------------------------------------------------------

(Instructions tested on ia64 FC10, paths may vary on others systems/archs).

Install mingw32 compiler:

sudo yum install mingw32-gcc

*** Compile libevent for Windows
Compile libevent for mingw32 and install it:

wget http://www.monkey.org/~provos/libevent-1.4.12-stable.tar.gz
tar xvzf libevent-1.4.12-stable.tar.gz

Edit configure.in and replace line that says AC_PROG_LIBTOOL by:

AC_LIBTOOL_DLOPEN
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL

Edit Makefile.in and add the replace the lines defining libevent_la_LDFLAGS,
libevent_core_la_LDFLAGS and libevent_extra_la_LDFLAGS by:

libevent_la_LDFLAGS = -release $(RELEASE) -version-info $(VERSION_INFO) -no-undefined
libevent_core_la_LDFLAGS = -release $(RELEASE) -version-info $(VERSION_INFO) -no-undefined
libevent_extra_la_LDFLAGS = -release $(RELEASE) -version-info $(VERSION_INFO) -no-undefined -levent

A patch file is included on e_libs distributions that performs these changes. It's located
on doc/libevent-1.4.12-mingw-dll.patch.

Then execute:
./autogen.sh --force
./configure --host=i686-pc-mingw32 --prefix=/usr/i686-pc-mingw32/sys-root/mingw/
make
sudo make install

Then copy the .dll files to the installation directory:
sudo cp .libs/*.dll /usr/i686-pc-mingw32/sys-root/mingw/lib/

*** Compile e_libs

./autogen.sh --host=i686-pc-mingw32 --prefix=/usr/i686-pc-mingw32/sys-root/mingw/
make
sudo make install
sudo cp src/.libs/*.dll /usr/i686-pc-mingw32/sys-root/mingw/lib/

-------------------------------------------------------------------------------
* BUILDING FOR OTHER TARGETS *
-------------------------------------------------------------------------------

* Building *
-------------------------------------------------------------------------------

Run:

./autogen.sh

make            # Compile
make check      # Run test cases
make install    # Install to default prefix ($HOME/tmp_install)

make clean              # Clean compiled
make maintainer-clean   # Full clean, you will need to run autogen.sh again


If you want to install to another prefix, run:

./autogen.sh
./configure --prefix=/your/wanted/prefix
make install

* Dependencies *
-------------------------------------------------------------------------------
Running autogen.sh will check for the required dependencies.

e_libs networking libraries depend on libevent to provide an async execution
model similar to Wavecom's OpenAT.

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
yum install gcc libtool autoconf automake check-devel splint valgrind gdb lcov libevent libevent-devel

For building documentation:
yum install doxygen texlive-latex

Source and debug info packages.
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
