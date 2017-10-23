/*=============================================================================
        Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

        This software is furnished under a license and may be used and copied
        only in accordance with the terms of such license and with the
        inclusion of the above copyright notice. This software or any other
        copies thereof may not be provided or otherwise made available to any
        other person. No title to and ownership of the software is hereby
        transferred.
==============================================================================*/
#ifndef _e_gps_flash_h_
#define _e_gps_flash_h_

#include "adl_global.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void e_gps_flash_init(void);
extern u8* e_gps_flash_default(void);
extern s32 e_gps_erase_flash(void);
extern s32 e_gps_read_flash(u8 *nv_data, u16 length);
extern s8 e_gps_write_flash(u8 *nv_data, u32 length);

#ifdef __cplusplus
}
#endif

#endif
