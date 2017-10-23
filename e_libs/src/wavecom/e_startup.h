/*=============================================================================
        Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

        This software is furnished under a license and may be used and copied
        only in accordance with the terms of such license and with the
        inclusion of the above copyright notice. This software or any other
        copies thereof may not be provided or otherwise made available to any
        other person. No title to and ownership of the software is hereby
        transferred.
==============================================================================*/
#ifndef __e_startup_h_
#define __e_startup_h_

#include "adl_global.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*e_startup_entry_point)(void);

extern adl_InitType_e e_startup_get_init_type(void);
extern void e_startup_main ( adl_InitType_e it, e_startup_entry_point ep, u32 delay);
extern void e_startup_cfg ( adl_InitType_e it, e_startup_entry_point cfg, e_startup_entry_point ep, u32 delay);

#ifdef __cplusplus
}
#endif

#endif
