DIRS := js css

all:
	for dir in $(DIRS); do $(MAKE) -C $$dir $@; done

build:
	for dir in $(DIRS); do $(MAKE) -C $$dir $@; done

check:
	for dir in $(DIRS); do $(MAKE) -C $$dir $@; done

install: all
	install -d -m 0750 $(HTMLDIR)/
	for dir in $(DIRS); do $(MAKE) -C $$dir $@; done

clean:
	for dir in $(DIRS); do $(MAKE) -C $$dir $@; done
