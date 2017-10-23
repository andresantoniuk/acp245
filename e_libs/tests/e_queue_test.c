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
#include "e_queue.h"

#include "e_check.h"

#define _MAX 200
START_TEST (test_e_queue_put_first)
{
    int d[_MAX];
    int i;
    e_queue q;

    e_queue_create(&q);

    NOT_NULL(q);
    IS_NULL(e_queue_head(q));
    IS_NULL(e_queue_tail(q));
    IS_NULL(e_queue_pop_first(q));
    IS_NULL(e_queue_pop_last(q));
    ARE_EQ_INT(0, e_queue_size(q));

    for (i = 0; i < _MAX; i++) {
        e_queue_put_first(q, &d[i]);
        ARE_SAME(&d[i], e_queue_head(q));
        ARE_SAME(&d[0], e_queue_tail(q));
        ARE_EQ_INT(i+1, e_queue_size(q));
    }

    for (i = 0; i < _MAX; i++) {
        int *foo;
        foo = e_queue_pop_last(q);
        ARE_SAME(&d[i], foo);
        if (i < (_MAX -1)) {
            ARE_SAME(&d[i+1], e_queue_tail(q));
            ARE_SAME(&d[_MAX -1], e_queue_head(q));
        } else {
            IS_NULL(e_queue_tail(q));
            IS_NULL(e_queue_head(q));
        }
        ARE_EQ_INT(_MAX-(i + 1), e_queue_size(q));
    }

    IS_NULL(e_queue_head(q));
    IS_NULL(e_queue_tail(q));
    IS_NULL(e_queue_pop_first(q));
    IS_NULL(e_queue_pop_last(q));
    ARE_EQ_INT(0, e_queue_size(q));

    for (i = 0; i < _MAX; i++) {
        e_queue_put_last(q, &d[i]);
        ARE_SAME(&d[i], e_queue_tail(q));
        ARE_SAME(&d[0], e_queue_head(q));
        ARE_EQ_INT(i+1, e_queue_size(q));
    }

    for (i = 0; i < _MAX; i++) {
        int *foo;
        foo = e_queue_pop_first(q);
        ARE_SAME(&d[i], foo);
        if (i < (_MAX - 1)) {
            ARE_SAME(&d[i+1], e_queue_head(q));
            ARE_SAME(&d[_MAX -1], e_queue_tail(q));
        } else {
            IS_NULL(e_queue_tail(q));
            IS_NULL(e_queue_head(q));
        }
        ARE_EQ_INT(_MAX-(i + 1), e_queue_size(q));
    }

    e_queue_free(&q);
    IS_NULL(q);
}
END_TEST

extern int e_queue_test (void);
int e_queue_test (void) {
    return e_check_run_suite("e_queue",
            test_e_queue_put_first,
            NULL);
}
#ifndef USE_SINGLE_TEST
int main (void) {
	return e_queue_test();
}
#endif
