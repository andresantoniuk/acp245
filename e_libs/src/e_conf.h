/*=============================================================================
        Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

        This software is furnished under a license and may be used and copied
        only in accordance with the terms of such license and with the
        inclusion of the above copyright notice. This software or any other
        copies thereof may not be provided or otherwise made available to any
        other person. No title to and ownership of the software is hereby
        transferred.
==============================================================================*/
#ifndef __e_conf_
#define __e_conf_

#include "e_port.h"
#include "e_splint_macros.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef E_LIBS_CONF_ENABLE

typedef struct _e_conf e_conf;

extern int e_conf_open(SPL_CONS_P e_conf** conf, const char* filename);

extern const char* e_conf_str(SPL_MSG_P e_conf* conf,
        const char* sec, const char* opt);

extern char* e_conf_str_c(SPL_MSG_P e_conf* conf,
        const char* sec, const char* opt);

extern int e_conf_int(SPL_MSG_P e_conf* conf,
        const char* section, const char* option, u32* v);

extern int e_conf_bool(SPL_MSG_P e_conf* conf,
        const char* section, const char* option, int* v);

extern void e_conf_free(SPL_DEST_P e_conf** conf);

extern int e_conf_def_set(const char* filename);

extern const char* e_conf_def_str(const char* sec, const char* opt);

extern int e_conf_def_int(const char* sec, const char* opt, u32* v);

extern void e_conf_def_free(void);

#endif /* E_LIBS_CONF_ENABLE */

#ifdef __cplusplus
}
#endif

#endif
