COMPRESSOR :=java -jar $(TOOLS)/yuicompressor-2.4.2.jar --type js
MIN_JS = acpyd_static.js

SCRIPTS = \
		jquery-1.3.2.min.js \
		jquery-ui-1.7.2.custom.min.js \
		jqDnR.js \
		grid.locale-en.js \
		grid.base.js \
		grid.common.js \
		grid.formedit.js \
		grid.custom.js \
		jquery.searchFilter.js \
		tree.js \
		jquery.treeview.js \
		base.js \
		gateway.js \
		testdb_grid.js

JSLINT_CHECK = base.js gateway.js tree.js testdb_grid.js
all: build

build: $(MIN_JS)

# Generalize this...
$(MIN_JS): $(SCRIPTS)
	rm -f $(MIN_JS);
	for x in $(SCRIPTS); do\
		cat $$x >> $(MIN_JS);\
		echo >> $(MIN_JS);\
	done
ifdef WEB_COMPRESS
	for x in $(MIN_JS); do\
		$(COMPRESSOR) $$x >> $$x.tmp;\
		mv $$x.tmp $$x;\
	done
endif

check: jslint

jslint:
	for x in $(JSLINT_CHECK); do\
		echo "Checking $$PWD/$$x";\
		java -classpath $(TOOLS)/custom_rhino.jar org.mozilla.javascript.tools.shell.Main $(TOOLS)/jslint.js $$x;\
	done

install: all
	install -d -m 0750 $(HTMLDIR)/js
	install -c -m 0640 $(MIN_JS) $(HTMLDIR)/js/

clean:
	rm -f $(MIN_JS).tmp $(MIN_JS)
