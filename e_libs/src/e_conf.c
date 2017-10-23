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

#include "e_conf.h"

#include <stdio.h>
#include <errno.h>

#include "e_port.h"
#include "e_util.h"
#include "e_mem.h"
#include "e_log.h"
#include "e_splint_macros.h"

#ifdef E_LIBS_CONF_ENABLE

#define _BUFF_SIZE  BUFSIZ

typedef struct _option option;
struct _option {
	char* name;
	char* value;

	option* next;
	option* prev;
};

typedef struct _section section;
struct _section {
	char* name;
	option* opt_list;

	section* next;
	section* prev;
};

struct _e_conf {
	char* filename;
	section* sec_list;
};

static option* _add_option(option** opt, char* name, char* val) {
	option* new;

	e_assert( name != NULL );
	e_assert( val != NULL );

	new = e_mem_malloc(sizeof(option));
	if( !new ) {
		E_LOG_NOMEM;
		return NULL;
	}
	memset(new, 0, sizeof(option));

	new->name = e_strdup(name);
	if( !new->name ) {
		e_mem_free(new);
		E_LOG_NOMEM;
		return NULL;
	}

	new->value = e_strdup(val);
	if( !new->name ) {
		e_mem_free(new->name);
		e_mem_free(new);
		E_LOG_NOMEM;
		return NULL;
	}
	if( !*opt ) {
		*opt = new;
	} else {
		option* tmp;
		tmp = (*opt)->next;
		(*opt)->next = new;
		new->prev = *opt;
		new->next = tmp;
	}

	E_DBG("Added option: %s=%s", new->name, new->value);

	return new;
}

static section* _add_section(section** sec, char* name) {
	section* new;

	e_assert( name != NULL );

	new = e_mem_malloc(sizeof(section));
	if( !new ) {
		E_LOG_NOMEM;
		return NULL;
	}
	memset(new, 0, sizeof(section));

	new->name = e_strdup(name);
	if( !new->name ) {
		e_mem_free(new);
		E_LOG_NOMEM;
		return NULL;
	}

	if( !*sec ) {
		*sec = new;
	} else {
		section* tmp;
		tmp = (*sec)->next;
		(*sec)->next = new;
		new->prev = *sec;
		new->next = tmp;
	}

	E_DBG("Added section: %s", new->name);

	return new;
}

static int _read_config(FILE* f, e_conf* conf) {
	u32 sz = _BUFF_SIZE;
	char* buf;
	u32 ls = 0;
	int ln;
	int rc;
	section* sec;
	option* opt;

	e_assert( f != NULL );
	e_assert( conf != NULL );

	buf = e_mem_malloc(sizeof(char) * sz);
	if( !buf ) {
		E_LOG_NOMEM;
		return -1;
	}

	ln = 0;
	rc = OK;
	sec = NULL;
	opt = NULL;
	while (rc >= 0 && (e_get_line(f, &buf, &sz, &ls) == OK)) {
		char* l;
		l = e_str_strip(buf, ls);
		if( !l ) {
			E_LOG_NOMEM;
			rc = -1;
			break;
		}
		ln++;
		if( e_strlen(l) == 0 || l[0] == '#' ) {
			goto next;
		} else if( l[0] == '[') {
			char* name;
			char* close;
			size_t name_l;
			close = strchr(l, ']');
			if( !close ) {
				E_ERR("Section doesn't close (line %d)", ln);
				rc = -1;
				goto next;
			}
			if( close == (l + 1) ) {
				E_ERR("Section without name (line %d)", ln);
				rc = -1;
				goto next;
			}
			name_l = close - (l+1);

			e_assert( name_l > 0 );

			name = e_mem_malloc(sizeof(char) * (name_l + 1));
			if( !name ) {
				E_LOG_NOMEM;
				rc = -1;
				goto next;
			}
			e_strncpy(name, l+1, (size_t)name_l);
			name[name_l] = '\0';

			sec = _add_section(&sec, name);

			e_mem_free(name);
            name = NULL;
			if( !sec ) {
				E_LOG_NOMEM;
				rc = -1;
				goto next;
			} else if( !conf->sec_list ) {
				conf->sec_list = sec;
			}

			opt = NULL;

		} else if( sec ) {
			char* name;
			char* vstr;
			char* p;
			p = strchr(l, '=');
			if( !p || (p == l)) {
				E_ERR("Not a section, option or comment (line %d)", ln);
				rc = -1;
				goto next;
			}
			e_assert( (p - l) >= 0 );
			e_assert(e_strlen(l) >= (u32) (p - l));
			name = e_str_strip(l, (u32) (p - l));

			vstr = e_str_strip(p + 1, (u32) e_strlen(p+1));

			opt  = _add_option(&opt, name, vstr);

			e_mem_free(name);
            name = NULL;
			e_mem_free(vstr);
            vstr = NULL;

			if( !opt ) {
				E_LOG_NOMEM;
				rc = -1;
				goto next;
			} else if( !sec->opt_list ) {
				sec->opt_list = opt;
			}
		} else {
			E_ERR("Text is not allowed outside a section (line %d)", ln);
			rc = -1;
			goto next;
		}
	next:
		e_mem_free(l);
        l = NULL;
	}

	if (rc == OK) {
		E_ERR("Error reading config file. (line %d)", ln);
	}

	e_mem_free(buf);
    buf = NULL;

	return rc;
}

static e_conf* _default_conf = NULL;

int
e_conf_def_set(const char* filename) {
	e_conf* new_conf = NULL;

	if( e_conf_open(&new_conf, filename) < 0 ) {
		return -1;
	}

	if( _default_conf ) {
		e_conf* tmp = _default_conf;
		_default_conf = new_conf;
		e_conf_free(&tmp);
	} else {
        _default_conf = new_conf;
    }

	return 0;
}

const char*
e_conf_def_str(const char* sec, const char* opt) {
	return _default_conf != NULL ? e_conf_str(_default_conf, sec, opt) : NULL;
}

int
e_conf_def_int(const char* sec, const char* opt, u32* v) {
	return _default_conf != NULL ? e_conf_int(_default_conf, sec, opt, v) : 0;
}

int e_conf_open(e_conf** conf, const char* filename) {
	FILE* f;

	e_assert( conf != NULL );
	e_assert( *conf == NULL );
	e_assert( filename != NULL );

	*conf = e_mem_malloc(sizeof(e_conf));
	if( !*conf ) {
		E_LOG_NOMEM;
		return -1;
	}

	memset(*conf, 0, sizeof(e_conf));

	(*conf)->filename = e_strdup(filename);
	if( !(*conf)->filename ) {
		E_LOG_NOMEM;
		e_conf_free(conf);
		return -1;
	}

	f = fopen((*conf)->filename, "r");
	if( !f ) {
		E_ERR("Error opening config file '%s': %s",
                (*conf)->filename, strerror(errno));
		e_conf_free(conf);
		return -1;
	}

	if( _read_config(f, *conf) < 0 ) {
		(void) fclose(f);
		e_conf_free(conf);
		return -1;
	}

	if( fclose(f) < 0 ) {
		E_ERR("Error closing file: %s", strerror(errno));
		e_conf_free(conf);
		return -1;
	}
	f = NULL;

	e_assert( *conf != NULL );
	return 0;
}

static section*
_find_section(e_conf* conf, const char* name) {
	section* cur;

	e_assert( conf != NULL );
	e_assert( name != NULL );

	cur = conf->sec_list;
	while( cur ) {
		if( strcmp(cur->name, name) == 0 ) {
			return cur;
		}
		cur = cur->next;
	}
	return NULL;
}

static option*
_find_option(section* sec, const char* name) {
	option* cur;

	e_assert( sec != NULL );
	e_assert( name != NULL );

	cur = sec->opt_list;
	while( cur ) {
		if( strcmp(cur->name, name) == 0 ) {
			return cur;
		}
		cur = cur->next;
	}
	return NULL;
}

const char* e_conf_str(e_conf* conf, const char* sn, const char* on) {
	section* sec;

	e_assert( conf != NULL );
	e_assert( sn != NULL );
	e_assert( on != NULL );

	sec = _find_section(conf, sn);

	if( sec ) {
		option* opt = _find_option(sec, on);
		if( opt ) {
			return opt->value;
		}
	}
	return NULL;
}

char* e_conf_str_c(e_conf* conf, const char* sn, const char* on) {
	const char* str;
	char* dup;
	str = e_conf_str(conf, sn, on);
	if( !str ) {
		return NULL;
	} 
	dup = e_strdup(str);
	if( !dup ) {
		E_LOG_NOMEM;
		return NULL;
	}
	return dup;
}

int e_conf_int(e_conf* conf, const char* sn, const char* on, u32* v) {
	const char* str;

	e_assert( v != NULL );

	str = e_conf_str(conf, sn, on);
	if( str ) {
		*v = (u32) e_atoi(str);
		return 0;
	} else {
		return -1;
	}
}
int
e_conf_bool(SPL_MSG_P e_conf* conf, const char* sn, const char* on, int* v) {
	const char* str;

	e_assert( v != NULL );

	str = e_conf_str(conf, sn, on);
	if( str ) {
		*v = (!strcmp(str, "true"))
            || (!strcmp(str, "yes"))
            || (!strcmp(str, "1")) ? 1 : 0;
		return 0;
	} else {
		return -1;
	}
}

void e_conf_free(SPL_DEST_P e_conf** conf) {
	section* sec;

	e_assert( conf != NULL );

	if( !*conf) {
		return;
	}

	sec = (*conf)->sec_list;
	while( sec ) {
		section* s_next;
		option* opt;

		opt = sec->opt_list;
		while( opt ) {
			option* o_next;

			o_next = opt->next;

			E_DBG("Freeing opt %s", opt->name);
			if( opt->name ) {
				e_mem_free(opt->name);
			}
			if( opt->value ) {
				e_mem_free(opt->value);
			}
			e_mem_free(opt);
			opt = o_next;
		}

		E_DBG("Freeing sec %s", sec->name);
		if( sec->name ) {
			e_mem_free(sec->name);
		}

		s_next = sec->next;
		e_mem_free(sec);
		sec = s_next;
	}
	if( (*conf)->filename ) {
		e_mem_free((*conf)->filename);
	}
	e_mem_free(*conf);
	*conf = NULL;
}

void e_conf_def_free(void) {
    e_conf_free(&_default_conf);
}

#endif /* E_LIBS_CONF_ENABLE */
