COVERAGE_DIR=coverage/

AM_CPPFLAGS = -I../src -I../src/linux-gnu

TESTS = \
	e_buff_test \
	e_conf_test \
	e_crc_test \
	e_log_test \
	e_port_test \
	e_queue_test \
	e_util_test

if E_LIBS_TIMER_ENABLE
TESTS += e_timer_test
e_timer_test_LDADD = -L../src/.libs/ -le_libs_check -le_libs -le_libs_utils
endif

if E_LIBS_NET_ENABLE
TESTS += e_syslog_test
e_syslog_test_LDADD = -L../src/.libs/ -le_libs_check -le_libs -le_libs_utils -le_libs_buff -le_libs_net
endif

CLEANFILES = \
	*.valgrind.out* \
	*.massif.* \
	*.gcno \
	*.gcda

clean-local:
	rm -rf $(COVERAGE_DIR)

MAINTAINERCLEANFILES = \
	Makefile.in

check_PROGRAMS = $(TESTS)

includedir = ${prefix}/include/e_libs
include_HEADERS= e_check_extra.h e_minicheck.h e_check.h
lib_LTLIBRARIES = libe_libs_check.la
libe_libs_check_c_sources = \
			    e_minicheck.c \
			    e_check_extra.c
libe_libs_check_h_sources = \
			    e_minicheck.h \
			    e_check.h \
			    e_check_extra.h
libe_libs_check_la_SOURCES = $(libe_libs_check_c_sources) $(libe_libs_check_h_sources)
libe_libs_check_la_LIBADD = -L../src/.libs/ -le_libs
libe_libs_check_la_LDFLAGS = $(event_LIBS) -no-undefined -version-info $(VERSION_INFO) -release $(VERSION) $(TARGET_LD_OPTS)

e_buff_test_LDADD = -L../src/.libs/ -le_libs_check -le_libs -le_libs_buff
e_conf_test_LDADD = -L../src/.libs/ -le_libs_check -le_libs -le_libs_utils -le_libs_conf
e_crc_test_LDADD = -L../src/.libs/ -le_libs_check -le_libs -le_libs_buff -le_libs_crc
e_log_test_LDADD = -L../src/.libs/ -le_libs_check -le_libs
e_port_test_LDADD = -L../src/.libs/ -le_libs_check -le_libs
e_queue_test_LDADD = -L../src/.libs/ -le_libs_check -le_libs -le_libs_utils
e_util_test_LDADD = -L../src/.libs/ -le_libs_check -le_libs -le_libs_utils

.PHONY: clean-coverage check-build valgrind massif valgrind-debug

VALGRINDCHECK=$(check_PROGRAMS)
VALGRINDOPTS=--log-file=$$x.valgrind.out

check-build:
	$(MAKE) $(AM_MAKEFLAGS) $(check_PROGRAMS)

valgrind: check-build
	rm -f *.valgrind.out.*
	export CK_FORK=no;
	export LD_LIBRARY_PATH=../src/.libs/;
	for x in $(VALGRINDCHECK); do \
		if [ -e ./.libs/$$x ]; then APP="./.libs/$$x"; else APP="./$$x"; fi; \
		CK_FORK=no LD_LIBRARY_PATH=../src/.libs/:.libs/ valgrind --leak-check=full --show-reachable=yes --track-fds=yes --error-exitcode=200 --num-callers=80 --leak-resolution=high --suppressions=./valgrind.supp $(VALGRINDOPTS) $$APP ; \
		if [ $$? -ne 0 ]; then \
			echo "Error running valgrind:memcheck!"; \
			cat $$x.valgrind.out*; \
			break; \
		fi; \
	done;

massif: check-build
	rm -f *.massif.stdout
	rm -f *.massif.dump
	export CK_FORK=no;
	export LD_LIBRARY_PATH=../src/.libs/;
	for x in $(VALGRINDCHECK); do \
		APP=$$x CK_FORK=no LD_LIBRARY_PATH=../src/.libs/ valgrind --tool=massif --stacks=yes --detailed-freq=1 --threshold=0.0 -v --log-file=$$x.massif.stdout --massif-out-file=$$x.massif.dump  --alloc-fn=e_mem_get ./.libs/$$x ; \
		if [ $$? -ne 0 ]; then \
			echo "Error running valgrind:massif!"; \
			cat $$x.massif.out*; \
			break; \
		fi; \
	done;

valgrind-debug: VALGRINDOPTS=--db-attach=yes
valgrind-debug: valgrind

coverage-zero:
	lcov -z -d .libs/ -d ../src/.libs/ -o $(COVERAGE_DIR)/coverage.lcov

coverage: coverage-zero check
	rm -rf $(COVERAGE_DIR)
	mkdir -p $(COVERAGE_DIR)
	for x in ../src/*.c; do ln -f $$x ../src/.libs; done
	lcov -c -d . -d ../src/ -o $(COVERAGE_DIR)/coverage.lcov
	genhtml -o $(COVERAGE_DIR)/ $(COVERAGE_DIR)/coverage.lcov
