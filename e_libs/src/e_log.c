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

#include "e_log.h"

#ifdef E_LIBS_LOG_ENABLE
#include "e_port.h"
#include "e_io.h"
#include "e_syslog.h"
#include "e_time.h"

int e_log_errlevel = E_LOG_INFO;
int e_log_facility = FACILITY_CONSOLE;

static int _init = 0;

static char _errors[E_BUFSIZ];
static char buf[E_BUFSIZ];
static char buf2[E_BUFSIZ];
static int _console_level = E_LOG_ALL;

static e_log_custom_handler _custom_handler = NULL;

/*@exposed@*/
static const char* _get_lvl_str(int lvl)
{
    switch(lvl) {
    case E_LOG_NONE:
        return "NONE";
    case E_LOG_FATAL:
        return "FATA";
    case E_LOG_ERR:
        return "ERRO";
    case E_LOG_WARN:
        return "WARN";
    case E_LOG_INFO:
        return "INFO";
    case E_LOG_DEBUG:
        return "DEBG";
    case E_LOG_TRACE:
        return "TRCE";
    case E_LOG_STACK:
        return "STAK";
    default:
        return "UNKN";
    }
}

static void _str_append(char** pos, const char* str, int *remain)
{
    int len;
    len = (int) e_strlen(str);
    if (len >= *remain) {
        len = *remain -1;
    }

    e_strncpy(*pos, str, (size_t) *remain);

    *remain -= len;
    *pos += len;
}

int e_log_get_level(void)
{
	return e_log_errlevel;
}

void e_log_set_level(int level)
{
	e_log_errlevel = level;
}
/* ad-hocish */
void e_log_set_console_level(int level) {
    _console_level = level;
}

int e_log_get_facility(void)
{
	return e_log_facility;
}

int e_log_set_facility(int f)
{
    if (f & ~(FACILITY_CUSTOM|FACILITY_CONSOLE|FACILITY_BUFFER)) {
        return -1;
    }
    e_log_facility = f;
    return 0;
}

int e_log_add_facility(int f)
{
    if (f & ~(FACILITY_CUSTOM|FACILITY_CONSOLE|FACILITY_BUFFER)) {
        return -1;
    }
    e_log_facility |= f;
    return 0;
}

void e_log_set_custom_handler(e_log_custom_handler handler) {
    _custom_handler = handler;
}

char*
e_log_get_err(void)
{
	if(!_init) {
		return e_strdup("");
	}
	return e_strdup(_errors);
}

static void _log_file_v(const char* file, const long line_number,
        int errlevel, const char *fmt, va_list* ap)
{
    int remain = E_BUFSIZ;
    time_t t;
    char* pos = buf;

    /* add timestamp */
    t = e_time_time(NULL);
    if (t == ((time_t)-1)) {
        _str_append(&pos, "0 ", &remain);
    } else {
        #ifdef HAVE_SNPRINF
        snprintf(buf2, E_BUFSIZ, "%lu ", (unsigned long) t);
        #else
        /*@-bufferoverflowhigh@*/
        e_sprintf(buf2, "%lu ", (unsigned long) t);
        /*@+bufferoverflowhigh@*/
        #endif
        _str_append(&pos, buf2, &remain);
    }

    /* add log level tag */
    _str_append(&pos, "[", &remain);
    _str_append(&pos, _get_lvl_str(errlevel), &remain);
    _str_append(&pos, "] ", &remain);

    /* add file name */
    if (file != NULL) {
        _str_append(&pos, file, &remain);
    }

    /* add line number */
	if( line_number >= 0 ) {
        #ifdef HAVE_SNPRINF
        snprintf(buf2, E_BUFSIZ, ":%ld - ", (long) line_number);
        #else
        /*@-bufferoverflowhigh@*/
        e_sprintf(buf2, ":%ld - ", (long) line_number);
        /*@+bufferoverflowhigh@*/
        #endif
        _str_append(&pos, buf2, &remain);
	}

    /* add message */
    #if defined(HAVE_VSNPRINTF) && !defined(__STRICT_ANSI__)
	(void)vsnprintf(buf2, E_BUFSIZ, fmt, *ap);
    #else
    /*@-bufferoverflowhigh@*/
	(void)vsprintf(buf2, fmt, *ap);
    /*@+bufferoverflowhigh@*/
    #endif
    _str_append(&pos, buf2, &remain);

    /* send log message to facility */
    if (e_log_facility & FACILITY_CUSTOM) {
        if (_custom_handler) {
            _custom_handler(errlevel, buf);
        }
    }

    if (e_log_facility & FACILITY_BUFFER) {
		_init = 1;
		(void) strlcpy(_errors, buf, E_BUFSIZ);
    }

    if ((e_log_facility & FACILITY_CONSOLE) && (errlevel <= _console_level)) {
        /* ATENTION: we are modifying buf2 here...*/
        _str_append(&pos, "\r\n", &remain);
        e_io_puts_stderr(buf);
    }
}

void e_log_file(const char* file, const long line, int errlevel,
        const char *fmt, ...)
{
	if (errlevel <= e_log_errlevel) {
		va_list ap;
		va_start(ap, fmt);
		_log_file_v(file, line, errlevel, fmt, &ap);
		va_end(ap);
	}
}

#if !((defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L ) \
        || (defined(__WM__)))
/* functions duplicate a little code to avoid calling an additional function
 * before doing errlevel check */
void E_LOG(int errlevel, const char *fmt, ...) {
	if (errlevel <= e_log_errlevel) {
		va_list ap;
		va_start(ap, fmt);
		_log_file_v("n/a", 0, errlevel, fmt, &ap);
		va_end(ap);
	}
}
void E_FATAL(const char *fmt, ...) {
	if (E_LOG_FATAL <= e_log_errlevel) {
		va_list ap;
		va_start(ap, fmt);
		_log_file_v("n/a", 0, E_LOG_FATAL, fmt, &ap);
		va_end(ap);
	}
}
void E_ERR(const char *fmt, ...) {
	if (E_LOG_ERR <= e_log_errlevel) {
		va_list ap;
		va_start(ap, fmt);
		_log_file_v("n/a", 0, E_LOG_ERR, fmt, &ap);
		va_end(ap);
	}
}
void E_WARN(const char *fmt, ...) {
	if (E_LOG_WARN <= e_log_errlevel) {
		va_list ap;
		va_start(ap, fmt);
		_log_file_v("n/a", 0, E_LOG_WARN, fmt, &ap);
		va_end(ap);
	}
}
void E_INFO(const char *fmt, ...) {
	if (E_LOG_INFO <= e_log_errlevel) {
		va_list ap;
		va_start(ap, fmt);
		_log_file_v("n/a", 0, E_LOG_INFO, fmt, &ap);
		va_end(ap);
	}
}
void E_DBG(const char *fmt, ...) {
	if (E_LOG_DEBUG <= e_log_errlevel) {
		va_list ap;
		va_start(ap, fmt);
		_log_file_v("n/a", 0, E_LOG_DEBUG, fmt, &ap);
		va_end(ap);
	}
}
void E_TRACE(const char *fmt, ...) {
	if (E_LOG_TRACE <= e_log_errlevel) {
		va_list ap;
		va_start(ap, fmt);
		_log_file_v("n/a", 0, E_LOG_TRACE, fmt, &ap);
		va_end(ap);
	}
}
void E_STACK(const char *fmt, ...) {
	if (E_LOG_STACK <= e_log_errlevel) {
		va_list ap;
		va_start(ap, fmt);
		_log_file_v("n/a", 0, E_LOG_STACK, fmt, &ap);
		va_end(ap);
	}
}
#endif

#else
/* C89 complains about empty .c files */
extern void* a;
#endif /* E_LIBS_LOG_ENABLE */
