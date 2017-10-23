/*=============================================================================
        Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

        This software is furnished under a license and may be used and copied
        only in accordance with the terms of such license and with the
        inclusion of the above copyright notice. This software or any other
        copies thereof may not be provided or otherwise made available to any
        other person. No title to and ownership of the software is hereby
        transferred.
==============================================================================*/
#ifndef __e_gps_h_
#define __e_gps_h_

#include "e_port.h"

#ifdef __cplusplus
extern "C" {
#endif

#define E_GPS_MAX_SAT               12

#define GPS_ERR_INIT_FAILED         0x1
#define GPS_ERR_FAILED              0x2

#define GPS_ERR_UART_CONF           (GPS_ERR_INIT_FAILED<<24| 1<<1)
#define GPS_ERR_CONF                (GPS_ERR_INIT_FAILED<<24| 2<<1)
#define GPS_ERR_FCM                 (GPS_ERR_INIT_FAILED<<24| 3<<1)
#define GPS_ERR_FCM_DATA            (GPS_ERR_INIT_FAILED<<24| 4<<1)
#define GPS_ERR_GPIO_CONF           (GPS_ERR_INIT_FAILED<<24| 5<<1)
#define GPS_ERR_GPS_START           (GPS_ERR_INIT_FAILED<<24| 6<<1)
#define GPS_ERR_GPS_CHECK           (GPS_ERR_INIT_FAILED<<24| 7<<1)

#define GPS_ERR_GPS_STOPPED         (GPS_ERR_FAILED<<24     | 1<<1)

typedef struct _e_gps_position
{
    bool            valid;
	double			lat;        /* degrees */
	double			lon;        /* degrees */
	double			altitude;   /* meters */
    u8              heading;    /* degrees */
    u8              speed;      /* meters/sec */
    double          error;
    u8              pdop;
    u8              vdop;
    u8              hdop;
    u8              tdop;
    u8              speed_m;
    u8              sats[E_GPS_MAX_SAT];
    u32             time;
} e_gps_position;

typedef void (*e_gps_pvt_handler)(void);

typedef void (*e_gps_err_handler)(s32);

typedef void (*e_gps_stop_handler)(void);

extern void e_gps_stop(e_gps_stop_handler handler);

extern e_gps_pvt_handler e_gps_get_pvt_handler(void);

extern void e_gps_register_pvt_handler(e_gps_pvt_handler handler);

extern void e_gps_unregister_pvt_handler(void);

extern e_gps_err_handler e_gps_get_err_handler(void);

extern void e_gps_register_err_handler(e_gps_err_handler handler);

extern void e_gps_unregister_err_handler(void);

e_gps_position *e_gps_get_current_fix(void);

#ifdef __cplusplus
}
#endif

#endif
