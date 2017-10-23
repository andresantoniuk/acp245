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
#ifndef __e_minicheck_h__
#define __e_minicheck_h__

#ifdef __cplusplus
extern "C" {
#endif

#define CK_NOFORK 0
#define CK_FORK   1

#define CK_SILENT  0
#define CK_NORMAL  1
#define CK_VERBOSE 2

/* Workaround for Tru64 Unix systems where the C compiler has a working
   __func__, but the C++ compiler only has a working __FUNCTION__.  This
   could be fixed in configure.in, but it's not worth it right now. */
#if defined(__osf__) && defined(__cplusplus)
#define __func__ __FUNCTION__
#endif

#define START_TEST(testname) static void testname(void) { \
    _check_set_test_info(__func__, __FILE__, __LINE__);   \
    {
#define END_TEST } }

#define fail(msg)  _fail_unless(0, __FILE__, __LINE__, msg)

typedef void (*tcase_setup_function)(void);
typedef void (*tcase_teardown_function)(void);
typedef void (*tcase_test_function)(void);

typedef struct SRunner SRunner;
typedef struct Suite Suite;
typedef struct TCase TCase;

struct SRunner {
    Suite *suite;
    int nchecks;
    int nfailures;
};

struct Suite {
    const char *name;
    TCase *tests;
};

struct TCase {
    const char *name;
    tcase_setup_function setup;
    tcase_teardown_function teardown;
    tcase_test_function *tests;
    int ntests;
    int allocated;
    TCase *next_tcase;
};

/* Internal helper. */
void _check_set_test_info(char const *function,
                          char const *filename, int lineno);


/*
 * Prototypes for the actual implementation.
 */

void fail_if(int condition, const char *file, int line, const char *expr, ...);
void fail_unless(int condition, const char *file, int line, const char *expr, ...);
Suite *suite_create(const char *name);
TCase *tcase_create(const char *name);
void suite_add_tcase(Suite *suite, TCase *tc);
void tcase_add_checked_fixture(TCase *,
                               tcase_setup_function,
                               tcase_teardown_function);
void tcase_add_test(TCase *tc, tcase_test_function test);
SRunner *srunner_create(Suite *suite);
void srunner_run_all(SRunner *runner, int verbosity);
int srunner_ntests_failed(SRunner *runner);
void srunner_free(SRunner *runner);

#ifdef __cplusplus
}
#endif
#endif /* __e_minicheck_h__ */
