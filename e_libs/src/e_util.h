/*=============================================================================
        Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

        This software is furnished under a license and may be used and copied
        only in accordance with the terms of such license and with the
        inclusion of the above copyright notice. This software or any other
        copies thereof may not be provided or otherwise made available to any
        other person. No title to and ownership of the software is hereby
        transferred.
==============================================================================*/
#ifndef _e_utils_
#define _e_utils_

#include "e_port.h"
#include <stdio.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

extern bool e_is_digit(ascii c);

/*@null@*/
extern char* e_std_date_now(void);

/*@null@*/
extern char* e_std_date(/*@in@*/ const time_t* t);

extern int e_file_access(/*@in@*/ const char* path);

/*@null@*/
extern char* e_str_strip(/*@in@*/ const char* str, size_t sz);

extern e_ret e_get_line(/*@in@*/ FILE* f, char** buf, u32 *n, u32 *readed);

/*@null@*/
extern ascii* e_util_to_hex(/*@null@*/ const u8* bytes, u32 size);

extern void e_util_print_hex(/*@null@*/ const u8* bytes, u32 size);

extern bool e_util_is_hex(ascii c);

extern u8 e_util_hex(ascii c);

extern e_ret e_util_to_bin(u8 *bin, const ascii *hexstr, u16 binsize);

#ifdef __cplusplus
}
#endif

#endif
