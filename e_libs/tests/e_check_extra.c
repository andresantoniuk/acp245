/*=============================================================================
        Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

        This software is furnished under a license and may be used and copied
        only in accordance with the terms of such license and with the
        inclusion of the above copyright notice. This software or any other
        copies thereof may not be provided or otherwise made available to any
        other person. No title to and ownership of the software is hereby
        transferred.
==============================================================================*/
#include "e_libs_config.h"
#include "e_check_extra.h"

#define DEFAULT_SEED    -1/*1232723221*/
#define DEFAULT_LEVEL   NONE

static int __seed = DEFAULT_SEED;
void e_check_set_seed(int seed) {
    __seed = seed;
}
int e_check_get_seed(void) {
    return __seed;
}
int e_check_run_suite(const char *name, ...) {
    Suite *s = suite_create (name);
    SRunner *sr;
    int number_failed;
    va_list ap;
    tcase_test_function func;
    TCase *tc_core;
    unsigned int seed;

    seed = __seed < 0 ? ((unsigned int) time(NULL)) :  (unsigned int) __seed;
    srand(seed);
    printf("Running %s. Random: %d\n", name, seed);

    E_LOG_SET_LEVEL(NONE);
    E_LOG_SET_FACILITY(CONSOLE);

    tc_core = tcase_create ("Core");
    va_start(ap, name);
    while((func = va_arg(ap, tcase_test_function)) != NULL) {
        tcase_add_test (tc_core, func);
    }
    va_end(ap);

    suite_add_tcase (s, tc_core);
    sr = srunner_create (s);

    srunner_run_all (sr, CK_NORMAL);
    number_failed = srunner_ntests_failed (sr);
    srunner_free (sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
