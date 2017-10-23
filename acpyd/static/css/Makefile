COMPRESSOR :=java -jar $(TOOLS)/yuicompressor-2.4.2.jar --type css
MIN_CSS = acpyd_static.css

COMPRESS = \
		   	jquery-ui-1.7.2.custom.css \
			ui.jqgrid.css \
			jquery.searchFilter.css \
			jquery.treeview.css \
			default.css \
		   	freeform.css \
		   	python.css \
		   	gateway.css \
		   	testdb.css
#grid.css

all: build

build: $(MIN_CSS)

$(MIN_CSS): $(COMPRESS)
	rm -rf $(MIN_CSS)
	for x in $(COMPRESS); do\
		cat $$x >> $(MIN_CSS);\
		echo >> $(MIN_CSS);\
	done
ifdef WEB_COMPRESS
	$(COMPRESSOR) $(MIN_CSS) >> $(MIN_CSS).tmp
	mv $(MIN_CSS).tmp $(MIN_CSS)
endif

install: all
	install -d -m 0750 $(HTMLDIR)/css
	install -c -m 0640 $(MIN_CSS) $(HTMLDIR)/css/
	install -d -m 0750 $(HTMLDIR)/css/images
	for file in images/*.*; do install -c -m 0640 $$file $(HTMLDIR)/css/images/; done;

clean:
	rm -f $(MIN_CSS).tmp $(MIN_CSS)

check:
