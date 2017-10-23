E_LIBS_LIBDIR=$(shell PKG_CONFIG_PATH=$(PKG_CONF_PATH):$(HOME)/tmp_install/lib/pkgconfig/ pkg-config --libs-only-L e_libs-1.0|sed -e 's,-L,,'|tr -d ' ')
MAINTAINERCLEANFILES = \
					   Makefile.in


.PHONY: gen

GENDIR = $(abs_srcdir)/gen

all: CFLAGS=
all: gen
	$(PYTHON) setup.py build

gen: src/c_pdu_gen.pxd src/pdu_gen.pyx

src/c_pdu_gen.pxd: \
	$(abs_top_srcdir)/src/acp_err.h \
	$(abs_top_srcdir)/src/acp_ie.h \
	$(abs_top_srcdir)/src/acp_el.h \
	$(abs_top_srcdir)/src/acp_msg.h \
	$(abs_top_srcdir)/src/acp_key.h \
	$(abs_top_srcdir)/src/acp_init.h
	-[ -f src/c_pdu_gen.header.pxd ] && cat src/c_pdu_gen.header.pxd > $@
	cd $(abs_top_srcdir);\
	python $(GENDIR)/generate.py --type pxd --output $(abs_srcdir)/$@ $+ --cppflags $(e_libs_CFLAGS) -DE_ACP245_HAVE_E_LIBS;
	-[ -f src/c_pdu_gen.footer.pxd ] && cat src/c_pdu_gen.footer.pxd >> $@

src/pdu_gen.pyx: \
	$(abs_top_srcdir)/src/acp_err.h \
	$(abs_top_srcdir)/src/acp_ie.h \
	$(abs_top_srcdir)/src/acp_el.h \
	$(abs_top_srcdir)/src/acp_msg.h \
	$(abs_top_srcdir)/src/acp_key.h \
	$(abs_top_srcdir)/src/acp_init.h
	-[ -f src/pdu_gen.header.pyx ] && cat src/pdu_gen.header.pyx > $@
	cd $(abs_top_srcdir);\
	python $(GENDIR)/generate.py --type pyx --output $(abs_srcdir)/$@ $+ --cppflags $(e_libs_CFLAGS) -DE_ACP245_HAVE_E_LIBS;
	-[ -f src/pdu_gen.footer.pyx ] && cat src/pdu_gen.footer.pyx >> $@

clean-local:
	rm -f src/c_pdu_gen.pxd src/pdu_gen.pyx
	rm -f gen/*.pyc
	$(PYTHON) setup.py clean
	rm -rf build/
	rm -f src/pdu_gen.c src/log.c
	rm -rf _trial_temp

pyrun: PY_LIBDIR=$(shell ls -d build/lib.*)
pyrun: all
	E_ACP245_LICENSE=$(top_srcdir)/tests/valid_license.sig LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(srcdir)/../src/.libs/:$(E_LIBS_LIBDIR) PYTHONPATH=$(PY_LIBDIR) $(PYTHON) $(RUN_OPTS)

install:
	$(PYTHON) setup.py install --root=$(DESTDIR) --prefix=${prefix}

install-strip: install
	find $(DESTDIR)/usr/lib -name "*.py" -print0 | xargs -0 rm

check-local: PY_LIBDIR=$(shell ls -d build/lib.*)
check-local: all
	E_ACP245_LICENSE=$(top_srcdir)/tests/valid_license.sig LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(E_LIBS_LIBDIR):../src/.libs/ PYTHONPATH=$(PY_LIBDIR) /usr/bin/trial `find $(PY_LIBDIR) -type f -name 'test_*.py'`

coverage: PY_LIBDIR=$(shell ls -d build/lib.*)
coverage: all
	coverage -e
	E_ACP245_LICENSE=$(top_srcdir)/tests/valid_license.sig LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(E_LIBS_LIBDIR):../src/.libs/ PYTHONPATH=$(PY_LIBDIR) coverage -o /usr/ -x /usr/bin/trial `find $(PY_LIBDIR) -type f -name 'test_*.py'`

test: check
