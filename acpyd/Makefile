BASEPATH    := $(CURDIR)
APPNAME     := acpyd
VERSION  	:= $(shell sed -ne 's/^%define[ \t]*_version[ \t]*\([0-9.]*\)$$/\1/p' $(APPNAME).spec)
DISTNAME 	:= $(APPNAME)-$(VERSION)
DESTDIR     := $(CURDIR)/dist/
RHROOT      := $(if $(wildcard ~/RH-BUILD), ~/RH-BUILD, /usr/src/redhat)

DIRS		:= static doc
HTMLDIR		:= $(DESTDIR)/var/$(APPNAME)/static/
TOOLS		:= $(CURDIR)/tools/

TMPDIRS		:= gen_console_scripts build _trial_temp dist doxygen vhost-skel vhost-logs vhosts

PYLINT		:= PYTHONPATH=build/lib/ pylint -i y --rcfile=pylint.rc

E_ACP245_LICENSE := $(BASEPATH)/test_license.sig

.EXPORT_ALL_VARIABLES:

.PHONY: all doc install install-strip clean test runserver tgz rpm

all: gen
	python setup.py build
	for dir in $(DIRS); do $(MAKE) -C $$dir $@; done

gen:
	rm -rf gen_console_scripts/
	mkdir gen_console_scripts/
	python gentools/gen_std_scripts.py gen_console_scripts/

doc:
	for dir in $(DIRS); do $(MAKE) -C $$dir $@; done

install:
	python setup.py install --root=$(DESTDIR)
	#rm -f `find twisted/ -type f -name 'dropin.cache' -o -name '.pyo'`
	install -d $(DESTDIR)/`ls -d /usr/lib/python*/site-packages/nevow`/plugins/
	cp -r nevow/plugins/* $(DESTDIR)/`ls -d /usr/lib/python*/site-packages/nevow`/plugins/
	if [ -d /usr/lib64/python*/site-packages/twisted ]; then \
		TWISTED_DIR=`ls -d /usr/lib64/python*/site-packages/twisted` \
		install -d $(DESTDIR)/`ls -d /usr/lib64/python*/site-packages/twisted`/plugins; \
		cp -r twisted/plugins/* $(DESTDIR)/`ls -d /usr/lib64/python*/site-packages/twisted`/plugins/; \
		rm -f $(DESTDIR)/`ls -d /usr/lib64/python*/site-packages/twisted`/plugins/dropin.cache;\
	else \
		TWISTED_DIR=`ls -d /usr/lib/python*/site-packages/twisted` \
		install -d $(DESTDIR)/`ls -d /usr/lib/python*/site-packages/twisted`/plugins; \
		cp -r twisted/plugins/* $(DESTDIR)/`ls -d /usr/lib/python*/site-packages/twisted`/plugins/; \
		rm -f $(DESTDIR)/`ls -d /usr/lib/python*/site-packages/twisted`/plugins/dropin.cache; \
	fi;
	install -d $(DESTDIR)/etc/init.d/
	install -m 755 etc/$(APPNAME).init $(DESTDIR)/etc/init.d/$(APPNAME)
	install -d $(DESTDIR)/etc/sysconfig
	install -m 644 etc/$(APPNAME).sysconfig $(DESTDIR)/etc/sysconfig/$(APPNAME)
	install -d $(DESTDIR)/etc/$(APPNAME)
	install -m 644 etc/$(APPNAME).conf $(DESTDIR)/etc/$(APPNAME)
	install -d $(DESTDIR)/usr/sbin/
	install -m 755 sbin/acpyd_respawn $(DESTDIR)/usr/sbin/
	install -d $(DESTDIR)/var/run/$(APPNAME)
	install -d $(DESTDIR)/var/log/$(APPNAME)
	install -d $(DESTDIR)/var/$(APPNAME)/
	install -d $(DESTDIR)/var/$(APPNAME)/js/
	cp -r js/*.js $(DESTDIR)/var/$(APPNAME)/js/
	install -d $(DESTDIR)/var/$(APPNAME)/templates/
	cp -r templates/*.html $(DESTDIR)/var/$(APPNAME)/templates/
	install -m 644 server.pem $(DESTDIR)/var/$(APPNAME)/
	install -d $(DESTDIR)/var/$(APPNAME)/gateway_scripts/
	install -d $(DESTDIR)/var/$(APPNAME)/gateway_scripts/server/
	cp -r gateway_scripts/server/*.py $(DESTDIR)/var/$(APPNAME)/gateway_scripts/server/
	install -d $(DESTDIR)/var/$(APPNAME)/gateway_scripts/client/
	cp -r gateway_scripts/client/*.py $(DESTDIR)/var/$(APPNAME)/gateway_scripts/client/
	install -d $(DESTDIR)/var/$(APPNAME)/vhosts/
	install -d $(DESTDIR)/var/$(APPNAME)/vhost-skel/
	install -d $(DESTDIR)/var/$(APPNAME)/vhost-skel/console_scripts/
	cp -r console_scripts/*.py $(DESTDIR)/var/$(APPNAME)/vhost-skel/console_scripts/
	install -d $(DESTDIR)/var/$(APPNAME)/std_console_scripts/
	cp -r gen_console_scripts/*.py $(DESTDIR)/var/$(APPNAME)/std_console_scripts/
	for dir in $(DIRS); do make -C $$dir $@; done

install-strip: install
	find $(DESTDIR)/usr/lib/python*/site-packages/$(APPNAME)/ -name "*.py" -print0 | xargs -0 rm

clean:
	python setup.py clean
	rm -f `find src/ -type f -name '*.pyc' -o -name '.pyo'`
	rm -f twisted/plugins/dropin.cache twisted/plugins/$(APPNAME)_plugin.pyc
	rm -f nevow/plugins/dropin.cache nevow/plugins/plugin.pyc
	for x in $(TMPDIRS); do rm -rf $$x; done
	for dir in $(DIRS); do make -C $$dir $@; done

jslint:
	$(MAKE) -C static/js $@

pylint: all
	$(PYLINT) acpyd$(TESTNAME)

check: all
	#PYTHONPATH=src/ trial acpyd.test$(TESTNAME)

runserver: all
	rm -rf vhosts
	rm -rf vhost-skel vhost-logs
	mkdir vhost-skel
	mkdir vhost-logs
	if [ -d testdb.console ]; then cp -a testdb.console vhost-skel/;fi
	cp -a console_scripts vhost-skel/
	cp -a gen_console_scripts/* vhost-skel/console_scripts/
	cp -a custom_scripts/* vhost-skel/console_scripts/
	PYTHONPATH=build/lib/ twistd -n $(APPNAME) --config=etc/acpyd.conf.test --log-dir=vhost-logs

runemuwx: all
	PYTHONPATH=build/lib/ python build/lib/acpyd/emuwx.py

runtest:
	rm -rf vhost-skel
	mkdir vhost-skel
	cp -a console_scripts vhost-skel/
	PYTHONPATH=build/lib/ python sbin/run_tests.py localhost 12004

gendoc: all
	PYTHONPATH=build/lib/ python sbin/gw_gen_doc.py gateway_scripts/ doc.html

# Alias, for backward compat.
test: check

tgz: clean
	install -d dist/$(DISTNAME)
	for target in $(filter-out dist build, $(wildcard *)); do \
		cp -r $$target dist/$(DISTNAME)/; \
	done
	cd dist; tar --exclude=.svn -czf $(DISTNAME).tar.gz $(DISTNAME)
	rm -fr dist/$(DISTNAME)

rpm: tgz
	rpmbuild -ta dist/$(DISTNAME).tar.gz
	rm -fr dist/*
	cp $(RHROOT)/RPMS/*/$(APPNAME)*-$(VERSION)-*.*.rpm dist/
	cp $(RHROOT)/SRPMS/$(APPNAME)*-$(VERSION)-*.*.rpm dist/
