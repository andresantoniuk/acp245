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

#include "e_crc.h"

#include "e_port.h"

/*
 * Based on code from Lammert Bies.
 * http://www.lammertbies.nl/comm/info/crc-calculation.html
 *
 */

#define _P_CCITT     0x1021

static u16 _tab_ccitt(u16 b) {
    int j;
    u16 crc, c;

    crc = 0;
    c   = b << 8;

    for (j = 0; j< 8; j++) {
        if ( (crc ^ c) & 0x8000 ) {
            crc = (( crc << 1 ) ^ _P_CCITT) & 0xFFFF;
        } else {
            crc = (crc << 1) & 0xFFFF;
        }
        c = (c << 1) & 0xFFFF;
    }

    return crc;
}

void e_crc_ccitt_init(u16 *table, u16 size) {
    e_assert(table != NULL);
    e_assert(size == 256);

    while(size--) {
        table[size] = _tab_ccitt(size);
    }
}

#define _CRC_CCITT_TAB(crc, b, tab) \
    (((crc) << 8) ^ (tab)[((crc) >> 8) ^ (0x00FF & (b))])
#define _CRC_CCITT(crc, b) \
    (((crc) << 8) ^ _tab_ccitt(((crc) >> 8) ^ (0x00FF & (b))))

static u16 _ccitt_buff_no_tab(u8* buff, u16 size) {
    u32 i;
    u16 crc = 0;

    e_assert(buff != NULL);

    for (i = 0; i < size; i++) {
        crc = (_CRC_CCITT(crc, buff[i])) & 0xFFFF;
    }
    return crc;
}

static u16 _ccitt_buff_with_tab(u8* buff, u16 size, u16* tab) {
    u32 i;
    u16 crc = 0;

    e_assert(buff != NULL);
    e_assert(tab != NULL);

    for (i = 0; i < size; i++) {
        crc = (_CRC_CCITT_TAB(crc, buff[i], tab)) & 0xFFFF;
    }
    return crc;
}

void e_crc_ccitt(u16 *crc, u8 b, u16* tab) {
    e_assert(crc != NULL);

    *crc = (tab ? _CRC_CCITT_TAB(*crc, b, tab) : _CRC_CCITT(*crc, b)) & 0xFFFF;
}

u16 e_crc_ccitt_buff(u8* buff, u16 size, u16* tab) {
    e_assert(buff != NULL);

    return tab ? _ccitt_buff_with_tab(buff, size, tab) :
        _ccitt_buff_no_tab(buff, size);
}

u16 e_crc_ccitt_ebuff(e_buff* buff, u32 from, u32 to, u16* tab) {
    u32 i;
    /*
    u32 o_pos;
    u32 o_lim;
    */
    u16 crc;

    e_assert (buff != NULL);
    e_assert (from >= 0 && from <= e_buff_capacity(buff));
    e_assert (to >= 0 && to <= e_buff_capacity(buff));
    e_assert (from <= to);

    /* accessing internal rep... will know if this was a good
     * idea in the future */

    /*
    o_pos = e_buff_get_pos(buff);
    o_lim = e_buff_get_lim(buff);
    */

    crc = 0;
    if(tab) {
        for (i = from; i < to; i++) {
            crc = (_CRC_CCITT_TAB(crc, buff->data[i], tab)) & 0xFFFF;
        }
    } else {
        for (i = from; i < to; i++) {
            crc = (_CRC_CCITT(crc, buff->data[i])) & 0xFFFF;
        }
    }

    /*
    e_buff_set_pos(o_pos);
    e_buff_set_lim(o_lim);
    */
    return crc;
}

