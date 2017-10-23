/*=============================================================================
        Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

        This software is furnished under a license and may be used and copied
        only in accordance with the terms of such license and with the
        inclusion of the above copyright notice. This software or any other
        copies thereof may not be provided or otherwise made available to any
        other person. No title to and ownership of the software is hereby
        transferred.
==============================================================================*/
#ifndef __e_units_h_
#define __e_units_h_

#include "e_port.h"

#ifdef __cplusplus
extern "C" {
#endif

extern u32 e_units_ms_to_kmh_i(u32 ms);
extern double e_units_ms_to_kmh(double ms);

extern u32 e_units_kmh_to_ms_i(u32 kmh);
extern double e_units_kmh_to_ms(double kmh);

#ifdef __cplusplus
}
#endif

#endif
