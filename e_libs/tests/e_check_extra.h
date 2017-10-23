/*=============================================================================
        Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

        This software is furnished under a license and may be used and copied
        only in accordance with the terms of such license and with the
        inclusion of the above copyright notice. This software or any other
        copies thereof may not be provided or otherwise made available to any
        other person. No title to and ownership of the software is hereby
        transferred.
==============================================================================*/
#ifndef __e_check_extra_h__
#define __e_check_extra_h__

#include "e_log.h"
#include "e_util.h"

/*#include <check.h>*/
#include "e_minicheck.h"
#include <math.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ARE_EQ_INT(xo, yo) \
{\
	long int __x = (long int)(xo); \
	long int __y = (long int)(yo);\
	fail_if((__x) != (__y),\
		__FILE__, __LINE__,\
		"\n\tExpected: %ld \n\tWas: %ld", \
		(long int)(__x), (long int) (__y));\
}
#define ARE_EQ_FLO(x, y) \
	fail_if((((x)-(y)) < 0.00001),\
		__FILE__, __LINE__,\
		"\n\tExpected: %f \n\tWas: %f", \
		(float)(x), (float) (y))
#define ARE_EQ_FLO_E(x, y, e) \
	fail_if((fabs((x)-(y)) > e), \
		__FILE__, __LINE__,\
		"\n\tExpected: %f \n\tWas: %f, Diff: %f Eps: %f", \
		(float)(x), (float) (y), (float) (fabs((x)-(y))), e)
#define ARE_EQ_DBL(x, y) \
	fail_if((((x)-(y)) < (double)(0.00001)), \
		__FILE__, __LINE__,\
		"\n\tExpected: %f \n\tWas: %f", \
		(double)(x), (double) (y))
#define ARE_EQ_DBL_E(x, y, e) \
	fail_if((fabs((x)-(y)) > e),\
	__FILE__, __LINE__,\
	"\n\tExpected: %f \n\tWas: %f, Diff: %f Eps: %f",\
	(double)(x), (double) (y), (fabs((x)-(y))), e)
#define ARE_EQ_STR(x, y) \
	fail_if((x) == NULL || (y) == NULL || strcmp((x), (y)) != 0,\
	__FILE__, __LINE__,\
	"\nExpected:\n'%s'\nWas:\n'%s'\n", (x), (y))
#define ARE_EQ_BINC(x, y, sz) \
{\
	int ___rc = memcmp((x), (y), sz);\
	if (___rc != 0) {\
		char* ___hx = e_util_to_hex((x),sz);\
		char* ___hy = e_util_to_hex((y),sz);\
		fail_if(TRUE, \
			__FILE__, __LINE__,\
			"\n\tMemory contents differ.\nExpected: \n'%s'\nWas:\n'%s'\n",\
			___hx, ___hy);free(___hx);\
		free(___hy);\
	}\
}
#define ARE_EQ_BIN(x, y, sz) \
	fail_if(memcmp((x), (y), sz) != 0,\
		__FILE__, __LINE__,\
		"\n\tMemory contents differ for size=%d.",\
		sz)
#define ARE_SAME(x, y) \
	fail_if((x) != (y),\
		__FILE__, __LINE__,\
		"\n\tNot pointing to the same address.")
#define IS_IN_STR(x, y) \
	fail_if((x) == NULL || (y) == NULL || strstr((x), (y)) == NULL,\
		__FILE__, __LINE__,\
		"\n\t '%s' is not in '%s'", (y), (x))
#define NOT_NULL(x)	\
	fail_if((x) == NULL,\
		__FILE__, __LINE__,\
		"\n\tExpected not NULL")
#define IS_NULL(x) \
	fail_if((x) != NULL, \
		__FILE__, __LINE__,\
		"\n\tExpected NULL")
#define IS_OK(x) \
	fail_if((x) < 0,\
		__FILE__, __LINE__,\
		"\n\tCall failed, expected value >= 0, got: %ld\n",\
		(long int) (x))
#define IS_NOT_OK(x) \
	fail_if((x) >= 0,\
		__FILE__, __LINE__,\
		"\n\tCall succeeded, expected value < 0, got: %ld\n",\
		(long int) (x))
#define IS_TRUE(x) \
	fail_if(!(x),\
		__FILE__, __LINE__,\
		"\n\tExpected true, got: %d\n",\
		(x))
#define IS_FALSE(x) \
	fail_if((x),\
		__FILE__, __LINE__,\
		"\n\tExpected false, got: %ld\n",\
		(long int) (x))
#define FAIL(x) \
	fail_if(TRUE,\
		__FILE__, __LINE__,\
		"\n\tFail: %s\n",(x))

#define CASE_PATH(x)	"cases/"x

extern void e_check_set_seed(int seed);
extern int e_check_get_seed(void);
extern int e_check_run_suite(const char *name, ...);

#ifdef __cplusplus
}
#endif

#endif /* __e_check_extra_h__ */
