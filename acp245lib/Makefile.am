ACLOCAL_AMFLAGS = -I m4

# default PKG_CONFIG_PATH for Edantech, used when automatically running
# autoconf when configure.ac is modified. This is also set in autogen.sh
export PKG_CONFIG_PATH=$(PKG_CONF_PATH):$(HOME)/tmp_install/lib/pkgconfig/

SUBDIRS = src tests python
DIST_SUBDIRS = src tests python doc

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = acp245-1.0.pc

E_LIBS_LIBDIR=$(shell PKG_CONFIG_PATH=$(PKG_CONF_PATH):$(HOME)/tmp_install/lib/pkgconfig/ pkg-config --libs-only-L e_libs-1.0|sed -e 's,-L,,'|tr -d ' ')
RHROOT=$(if $(wildcard ~/RH-BUILD), ~/RH-BUILD, /usr/src/redhat)

MAINTAINERCLEANFILES = \
        aclocal.m4 \
        config.guess \
        *_config.h.in* \
        config.sub \
        configure \
        epcomp \
        install-sh \
        ltmain.sh \
        Makefile.in \
		depcomp \
        missing

docs:
	doxygen ./doxygen/doxyfile
#	cp ./doxygen/doxygen.sty ./doxygen/latex
	$(MAKE) -C ./doxygen/latex

gen:
	$(MAKE) -C python gen

massif: all
	$(MAKE) -C tests massif

callgrind: all
	$(MAKE) -C tests callgrind

valgrind: all
	$(MAKE) -C tests valgrind

valgrind-debug: all
	$(MAKE) -C tests valgrind-debug

coverage: all
	$(MAKE) -C tests coverage

check-build: all
	$(MAKE) -C tests check-build

pytest: all
	$(MAKE) -C python check

pyrun:
	$(MAKE) -C python pyrun

clean-profile:
	rm -f `find $(top_srcdir) -type f -name "*.gcda"`

clean-local:
	rm -rf dist/
	rm -rf doxygen/latex doxygen/html doxygen/rtf

test: clean-profile check

splint:
	$(MAKE) -C src splint

# Wavecom specific targets
wm_%:
	$(MAKE) -f Makefile.wm $*

# Windows specific targets
win_%:
	$(MAKE) -f Makefile.win $*

tgz: clean
	rm -rf dist/$(distdir)
	install -d dist/$(distdir)
	for target in $(filter-out .svn dist $(MAINTAINERCLEANFILES), $(wildcard *)); do \
		cp -r -l $$target dist/$(distdir)/; \
	done
	cd dist; tar --exclude=.svn -czf $(distdir).tar.gz $(distdir)
	rm -fr dist/$(distdir)

rpm: tgz
	rpmbuild -ta dist/$(distdir).tar.gz
	rm -fr dist/*
	cp $(RHROOT)/RPMS/*/$(PACKAGE)*-$(VERSION)*.rpm dist/
	cp $(RHROOT)/SRPMS/$(PACKAGE)-$(VERSION)-*.*.rpm dist/

rpm-install: rpm
	sudo rpm -Uvh --force dist/$(PACKAGE)*-$(VERSION)*.rpm
