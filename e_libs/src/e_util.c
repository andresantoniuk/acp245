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

#include "e_util.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef HAS_ACCESS
#include <unistd.h>
#endif

#include "e_port.h"
#include "e_mem.h"

#if defined(HAVE_TLS)
#   define __thread__   __thread
#else
#   define __thread__
#endif

E_INLINE bool
e_is_digit(ascii c)
{
    return (c >= 0x30) && (c <= 0x39);
}

/*@null@*/
char*
e_std_date_now(void)
{
	time_t t;

	(void) time(&t);

	return e_std_date(&t);
}

static __thread__ char _std_date[20];

/*@null@*/
char*
e_std_date(const time_t* t_arg)
{
	struct tm* t;

	t = gmtime(t_arg);
	if( !t ) {
		return NULL;
	}
    #if defined(HAVE_SNPRINTF) && !defined(__STRICT_ANSI__)
	snprintf(_std_date,
		sizeof(_std_date),
		"%.4d-%.2d-%.2d %.2d:%.2d:%.2d",
		1900 + t->tm_year, t->tm_mon + 1, t->tm_mday,
		t->tm_hour, t->tm_min, t->tm_sec);
    #else
    /* assumes gmtime returns values according to spec! */
    /*@-bufferoverflowhigh@*/
	sprintf(_std_date,
		"%.4d-%.2d-%.2d %.2d:%.2d:%.2d",
		(1900 + (t->tm_year&0xfff)), t->tm_mon + 1, t->tm_mday,
		t->tm_hour, t->tm_min, t->tm_sec);
    /*@+bufferoverflowhigh@*/
    #endif

	return _std_date;
}

int
e_file_access(const char* path) {
#ifdef HAS_ACCESS
	return access(path, F_OK) == 0 ? 1 : 0;
#else
	FILE *fp = fopen(path, "r");
	if( fp ) {
		(void) fclose(fp);
		return 1;
	} else {
		return 0;
	}
#endif
}

/*@null@*/
char*
e_str_strip(const char* str, size_t sz) {
	char* b;
	char* strip;
	size_t i;
	const char* start;
	size_t len;

	e_assert( (str != NULL) || (sz == 0));

	b = e_mem_malloc(sizeof(char) * (sz+1));
	if( !b ) {
		return NULL;
	}
    (void) e_mem_set(b, 0, sizeof(char) * (sz+1));

	len = sz;
	start = str;

	for( i = 0 ; i < sz && str[i] == ' ' ; i++ );
	start += i;
	len   -= i;

	if( len > 0 ) {
		for( i = sz - 1 ; i >= 0 && str[i] == ' ' ; i-- );
		len = len - ((sz-1)-i);
	}

	e_assert (len <= sz);

	e_strncpy(b, start, (size_t)len);
	b[len] = '\0';

	strip = e_strdup(b);
	e_mem_free(b);
	return strip;
}

e_ret
e_get_line(FILE* f, char** buf, u32 *n, u32 *readed) {
    u32 r = 0;
	int c;

	e_assert( f != NULL );
	e_assert( buf != NULL && *buf != NULL );
	e_assert( n != NULL && *n > 1 );

	while( ((c = fgetc(f)) != EOF) && c != (int)'\n' ) {
		(*buf)[r++] = (char)c;
		if( r == (*n -1) ) {
			char* n_buf;
			n_buf = realloc(*buf, (size_t) (*n + 1024));
			if( n_buf ) {
				*buf = n_buf;
				*n   = *n + 1024;
			} else {
				return ERROR;
			}
		}
	}

	if( c == EOF ) {
		return ERROR;
	} else if( c == (int)'\n' ) {
		(*buf)[r] = '\0';
	}

    *readed = r;
	return OK;
}

/*@null@*/
ascii*
e_util_to_hex(/*@null@*/ const u8* bytes, u32 size)
{
	static char hexval[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
	u32 i;
	char *hex;

	e_assert( (bytes != NULL) || (size == 0) );

	if( !bytes ) {
		return NULL;
	}

	hex = e_mem_malloc(sizeof(ascii)*(size*2 + 1));
	if( !hex ) {
		return NULL;
	}

	for( i = 0 ; i < size ; i++ ){
		hex[i*2] = hexval[(int)((bytes[i] >> 4) & 0xF)];
		hex[(i*2) + 1] = hexval[(int)(bytes[i]) & 0xF];
	}
	hex[size*2] = '\0';

	e_assert( hex != NULL );

	return hex;
}

void
e_util_print_hex(/*@null@*/ const u8* bytes, u32 size)
{
	ascii* hex = e_util_to_hex(bytes, size);
	if (hex) {
		printf("%s", hex);
		e_mem_free(hex);
	} else {
		printf("NULL");
	}
}

E_INLINE bool e_util_is_hex(ascii c) {
    return (c >= 0x30 && c <= 0x39) || (c >= 0x61 && c <= 0x66);
}

E_INLINE u8 e_util_hex(ascii c) {
    return (c > 0x60 ? c - 0x57 : c - 0x30) & 0xFFu;
}

e_ret e_util_to_bin(u8 *bin, const ascii *hexstr, u16 binsize) {
    u8 i;

    e_assert(hexstr != NULL);
    e_assert(bin != NULL);
    e_assert(e_strlen(hexstr) >= (binsize * 2u));

    for (i = 0; i < binsize; i++) {
        ascii c1;
        ascii c2;
        c1 = hexstr[i*2];
        c2 = hexstr[i*2+1];
        if (!e_util_is_hex(c1) || !e_util_is_hex(c2)) {
            return ERROR;
        }
        bin[i] = (((e_util_hex(c1) & 0xFu) << 4u) | (e_util_hex(c2) & 0xFu)) & 0xFFu;
    }

    return OK;
}
