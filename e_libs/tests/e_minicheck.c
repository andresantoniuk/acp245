/*
 * Code based on the check unit framework for C, based on
 * modifications made by the expat project.
 *
 * Expat copyright:
 *
 * Copyright (c) 1998, 1999, 2000 Thai Open Source Software Center Ltd
 * See the file COPYING for copying permission.
 *
 * Check Unit test framework Copyright:
 *
 * Check: a unit test framework for C
 * Copyright (C) 2001, 2002, Arien Malec
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/* Miniature re-implementation of the "check" library.
 * This is *source* compatible, but not necessary *link* compatible.
 */
#include "e_libs_config.h"
#include "e_libs_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <assert.h>
#include <stdarg.h>

#include "e_minicheck.h"

Suite *
suite_create(const char *name)
{
    Suite *suite = (Suite *) calloc(1, sizeof(Suite));
    if (suite != NULL) {
        suite->name = name;
        suite->tests = NULL;
    }
    return suite;
}

TCase *
tcase_create(const char *name)
{
    TCase *tc = (TCase *) calloc(1, sizeof(TCase));
    if (tc != NULL) {
        tc->name = name;
        tc->tests = NULL;
    }
    return tc;
}

void
suite_add_tcase(Suite *suite, TCase *tc)
{
    assert(suite != NULL);
    assert(tc != NULL);
    assert(tc->next_tcase == NULL);

    tc->next_tcase = suite->tests;
    suite->tests = tc;
}

void
tcase_add_checked_fixture(TCase *tc,
                          tcase_setup_function setup,
                          tcase_teardown_function teardown)
{
    assert(tc != NULL);
    tc->setup = setup;
    tc->teardown = teardown;
}

void
tcase_add_test(TCase *tc, tcase_test_function test)
{
    assert(tc != NULL);
    if (tc->allocated == tc->ntests) {
        int nalloc = tc->allocated + 100;
        size_t new_size = sizeof(tcase_test_function) * nalloc;
        tc->tests = realloc(tc->tests, new_size);
        assert(tc->tests != NULL);
        tc->allocated = nalloc;
    }
    tc->tests[tc->ntests] = test;
    tc->ntests++;
}

SRunner *
srunner_create(Suite *suite)
{
    SRunner *runner = calloc(1, sizeof(SRunner));
    if (runner != NULL) {
        runner->suite = suite;
    }
    return runner;
}

static jmp_buf env;

static char const *_check_current_function = NULL;
static int _check_current_lineno = -1;
static char const *_check_current_filename = NULL;

void
_check_set_test_info(char const *function, char const *filename, int lineno)
{
    _check_current_function = function;
    _check_current_lineno = lineno;
    _check_current_filename = filename;
}


static void
add_failure(SRunner *runner, int verbosity)
{
    runner->nfailures++;
    if (verbosity >= CK_VERBOSE) {
        printf("%s:%d: %s\n", _check_current_filename,
               _check_current_lineno, _check_current_function);
    }
}

void
srunner_run_all(SRunner *runner, int verbosity)
{
    Suite *suite;
    TCase *tc;
    assert(runner != NULL);
    suite = runner->suite;
    tc = suite->tests;
    while (tc != NULL) {
        int i;
        for (i = 0; i < tc->ntests; ++i) {
            runner->nchecks++;

            if (tc->setup != NULL) {
                /* setup */
                if (setjmp(env)) {
                    add_failure(runner, verbosity);
                    continue;
                }
                tc->setup();
            }
            /* test */
            if (setjmp(env)) {
                add_failure(runner, verbosity);
                continue;
            }
            (tc->tests[i])();

            /* teardown */
            if (tc->teardown != NULL) {
                if (setjmp(env)) {
                    add_failure(runner, verbosity);
                    continue;
                }
                tc->teardown();
            }
        }
        tc = tc->next_tcase;
    }
    if (verbosity) {
        int passed = runner->nchecks - runner->nfailures;
        double percentage;
        int display;
        if (runner->nchecks > 0) {
            percentage = ((double) passed) / runner->nchecks;
        } else {
            percentage = 0;
        }
        display = (int) (percentage * 100);
        printf("%d%%: Checks: %d, Failed: %d\n",
               display, runner->nchecks, runner->nfailures);
    }
}

void
fail_unless(int condition, const char *file, int line, const char *expr, ...)
{
    char buf[BUFSIZ];
    if (!condition) {
        va_list ap;
        va_start(ap,expr);
        #if defined HAVE_VSNPRINTF && !defined __STRICT_ANSI__
        (void)vsnprintf(buf, BUFSIZ, expr, ap);
        #else
        /*@-bufferoverflowhigh@*/
        (void)vsprintf(buf, expr, ap);
        /*@+bufferoverflowhigh@*/
        #endif
        va_end(ap);
        printf("%s:%d -> %s\n", file, line, buf);
        longjmp(env, 1);
    }
}
void
fail_if(int condition, const char* file, int line, const char *expr, ...)
{
    char buf[BUFSIZ];
    if (condition) {
        va_list ap;
        va_start(ap,expr);
        #if defined HAVE_VSNPRINTF && !defined __STRICT_ANSI__
        (void)vsnprintf(buf, BUFSIZ, expr, ap);
        #else
        /*@-bufferoverflowhigh@*/
        (void)vsprintf(buf, expr, ap);
        /*@+bufferoverflowhigh@*/
        #endif
        va_end(ap);
        printf("%s:%d -> %s\n", file, line, buf);
        longjmp(env, 1);
    }
}

int
srunner_ntests_failed(SRunner *runner)
{
    assert(runner != NULL);
    return runner->nfailures;
}

void
srunner_free(SRunner *runner)
{
    TCase *test;
    TCase *tmp;
    if (runner  && runner->suite) {
        test = runner->suite->tests;
        while (test != NULL) {
            free(test->tests);
            tmp = test;
            test = test->next_tcase;
            free(tmp);
        }
    }
    free(runner->suite);
    free(runner);
}
