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

#include "e_gps.h"

#include "e_mem.h"
#include "e_log.h"

static e_gps_pvt_handler _pvt_handler = NULL;
static e_gps_err_handler _err_handler = NULL;
static e_gps_position _curr_pos;

void e_gps_stop(e_gps_stop_handler handler) {
    handler();
}

e_gps_pvt_handler e_gps_get_pvt_handler(void) {
    return _pvt_handler;
}

void e_gps_register_pvt_handler(e_gps_pvt_handler handler) {
    _pvt_handler = handler;
}

void e_gps_unregister_pvt_handler(void) {
    _pvt_handler = NULL;
}

e_gps_err_handler e_gps_get_err_handler(void) {
    return _err_handler;
}

void e_gps_register_err_handler(e_gps_err_handler handler) {
    _err_handler = handler;
}

void e_gps_unregister_err_handler(void) {
    _err_handler = NULL;
}

e_gps_position *e_gps_get_current_fix(void) {
    u8 i;
    e_mem_set(&_curr_pos, 0, sizeof(_curr_pos));
    _curr_pos.valid = TRUE;
    _curr_pos.lat = -30;
    _curr_pos.lon = -50;
	_curr_pos.altitude = 0;
    _curr_pos.heading = 0;
    _curr_pos.speed = 0;
    _curr_pos.pdop = 0;
    _curr_pos.vdop = 0;
    _curr_pos.hdop = 0;
    _curr_pos.tdop = 0;

    for (i = 0; i < E_GPS_MAX_SAT; i++) {
        _curr_pos.sats[i] = i;
    }
    E_DBG("Got dummy GPS pos");
    return &_curr_pos;
}
