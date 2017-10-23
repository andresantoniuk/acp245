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

#include "e_units.h"

#include "e_port.h"

u32 e_units_ms_to_kmh_i(u32 ms) {
    return (u32) (ms*3.6);
}
double e_units_ms_to_kmh(double ms) {
    return ms*3.6;
}

u32 e_units_kmh_to_ms_i(u32 kmh) {
    return (u32) 3.6/kmh;
}
double e_units_kmh_to_ms(double kmh) {
    return 3.6/kmh;
}
