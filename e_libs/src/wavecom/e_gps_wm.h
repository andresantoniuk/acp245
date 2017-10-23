/*=============================================================================
        Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

        This software is furnished under a license and may be used and copied
        only in accordance with the terms of such license and with the
        inclusion of the above copyright notice. This software or any other
        copies thereof may not be provided or otherwise made available to any
        other person. No title to and ownership of the software is hereby
        transferred.
==============================================================================*/
#ifndef __e_gps_wm_h_
#define __e_gps_wm_h_

#include "e_gps.h"
#include "adl_global.h"
#include "eRide.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WM_FASTRACK

#define E_GPS_GPIO_NRST     (ADL_IO_GPIO | 19);
#define E_GPS_GPIO_GPS      (ADL_IO_GPIO | 22);
#define E_GPS_GPIO_TCXO     (ADL_IO_GPIO | 23);
#define E_GPS_GPIO_LED      (ADL_IO_GPIO | 20);

#elif defined(WM_MOBIPOWER)

#define E_GPS_GPIO_NRST     (ADL_IO_GPIO | 19);
#define E_GPS_GPIO_GPS      (ADL_IO_GPIO | 22);
#define E_GPS_GPIO_TCXO     (ADL_IO_GPIO | 23);
#define E_GPS_GPIO_LED      (ADL_IO_GPIO | 20);

#endif

extern s32 e_gps_start(adl_port_e gps_uart);

#ifdef __cplusplus
}
#endif

#endif
