COVERAGE_DIR=coverage/
TESTS = \
	acp_el_test \
	acp_msg_test \
	acp_key_test \
	acp_license_test

CLEANFILES = \
	*.valgrind.out* \
	*.massif.* \
	*.callgrind.* \
	callgrind.* \
	*.gcno \
	*.gcda

MAINTAINERCLEANFILES = \
	Makefile.in

check_PROGRAMS = $(TESTS)

AM_CPPFLAGS = -I../src -I../src/linux-gnu $(e_libs_CFLAGS) $(e_libs_buff_CFLAGS) $(e_libs_check_CFLAGS) $(e_libs_utils_CFLAGS)

acp_msg_test_LDADD = -L../src/.libs/ -lacp245 $(e_libs_check_LIBS) $(e_libs_utils_LIBS) $(e_libs_buff_LIBS) $(e_libs_LIBS)
acp_el_test_LDADD = -L../src/.libs/ -lacp245 $(e_libs_check_LIBS) $(e_libs_utils_LIBS) $(e_libs_buff_LIBS) $(e_libs_LIBS)
acp_key_test_LDADD = -L../src/.libs/ -lacp245 $(e_libs_check_LIBS) $(e_libs_utils_LIBS) $(e_libs_buff_LIBS) $(e_libs_LIBS)
acp_license_test_LDADD = -L../src/.libs/ -lacp245 $(e_libs_check_LIBS) $(e_libs_utils_LIBS) $(e_libs_buff_LIBS) $(e_libs_LIBS)

clean-local:
	rm -rf $(COVERAGE_DIR)

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
		CK_FORK=no LD_LIBRARY_PATH=../src/.libs/ valgrind --leak-check=full --show-reachable=yes --track-fds=yes --error-exitcode=200 --num-callers=80 --leak-resolution=high --suppressions=./valgrind.supp $(VALGRINDOPTS) $$APP ; \
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

callgrind: check-build
	rm -f *.callgrind.stdout.*
	rm -f callgrind.out.*
	export CK_FORK=no;
	export LD_LIBRARY_PATH=../src/.libs/;
	for x in $(VALGRINDCHECK); do \
		APP=$$x CK_FORK=no LD_LIBRARY_PATH=../src/.libs/ valgrind --tool=callgrind --log-file=$$x.callgrind.stdout ./.libs/$$x ; \
		if [ $$? -ne 0 ]; then \
			echo "Error running valgrind:callgrind!"; \
			cat $$x.callgrind.out.*; \
			break; \
		fi; \
	done;

valgrind-debug: VALGRINDOPTS=--db-attach=yes
valgrind-debug: valgrind

coverage-zero:
	lcov -z -d .libs/ -d ../src/.libs/ -o $(COVERAGE_DIR)/acp.lcov

coverage: coverage-zero check
	rm -rf $(COVERAGE_DIR)
	mkdir -p $(COVERAGE_DIR)
	for x in ../src/*.c; do ln -f $$x ../src/.libs; done
	lcov -c -d . -d ../src/ -o $(COVERAGE_DIR)/acp.lcov
	genhtml -o $(COVERAGE_DIR)/ $(COVERAGE_DIR)/acp.lcov
