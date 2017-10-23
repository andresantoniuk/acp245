ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src doc tests
DIST_SUBDIRS = src doc tests

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = e_libs-1.0.pc e_libs_buff-1.0.pc e_libs_utils-1.0.pc e_libs_conf-1.0.pc e_libs_crc-1.0.pc e_libs_net-1.0.pc e_libs_check-1.0.pc 

DISTNAME=$(PACKAGE)-$(VERSION)
RHROOT      := $(if $(wildcard ~/RH-BUILD), ~/RH-BUILD, /usr/src/redhat)

MAINTAINERCLEANFILES = \
        aclocal.m4 \
        config.guess \
        *_config.h.in* \
        config.sub \
        configure \
        depcomp \
        install-sh \
        ltmain.sh \
        Makefile.in \
        dist \
        INSTALL \
        missing

BUILT_SOURCES = e_libs.h

e_libs.h: e_libs.h-head e_libs.h-tail e_libs_config.h
	cat $(srcdir)/e_libs.h-head >e_libs.h
	grep "define VERSION"  e_libs_config.h | sed -e 's,VERSION,E_LIBS_VERSION,' >> e_libs.h
	grep ENABLE e_libs_config.h >>e_libs.h
	cat $(srcdir)/e_libs.h-tail >>e_libs.h

docs: DOXY_VERSION=$(shell doxygen, --version)
docs:
	doxygen ./doxygen/doxyfile
#cp ./doxygen/doxygen.sty ./doxygen/latex
	$(MAKE) -C ./doxygen/latex

massif: all
	$(MAKE) -C tests massif

valgrind: all
	$(MAKE) -C tests valgrind

valgrind-debug: all
	$(MAKE) -C tests valgrind-debug

coverage: all
	$(MAKE) -C tests coverage

clean-coverage:
	rm -f `find $(top_builddir) -type f -name '*.gcda'`

check-build: all
	$(MAKE) -C tests check-build

clean-profile:
	rm -f `find $(top_srcdir) -type f -name "*.gcda"`

clean-local:
	rm -rf dist/
	rm -rf doxygen/latex doxygen/html doxygen/rtf

test: clean-profile check

win_%:
	make -f Makefile.win $*

wm_%:
	make -f Makefile.wm $*

splint: all
	$(MAKE) -C src splint

tgz: clean
	rm -rf dist/$(DISTNAME)
	install -d dist/$(DISTNAME)
	for target in $(filter-out .svn dist $(MAINTAINERCLEANFILES), $(wildcard *)); do \
		cp -r -l $$target dist/$(DISTNAME)/; \
	done
	cd dist; tar --exclude=.svn -czf $(DISTNAME).tar.gz $(DISTNAME)
	rm -fr dist/$(DISTNAME)

rpm: tgz
	rpmbuild -ta dist/$(DISTNAME).tar.gz
	rm -fr dist/*
	cp $(RHROOT)/RPMS/*/$(PACKAGE)*-$(VERSION)*.rpm dist/
	cp $(RHROOT)/SRPMS/$(PACKAGE)-$(VERSION)-*.*.rpm dist/

rpm-install: rpm
	sudo rpm -Uvh --force dist/$(PACKAGE)*-$(VERSION)*.rpm
