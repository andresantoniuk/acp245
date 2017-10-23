/*=============================================================================
        Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

        This software is furnished under a license and may be used and copied
        only in accordance with the terms of such license and with the
        inclusion of the above copyright notice. This software or any other
        copies thereof may not be provided or otherwise made available to any
        other person. No title to and ownership of the software is hereby
        transferred.
==============================================================================*/
#ifndef __e_crc_h_
#define __e_crc_h_

#include "e_port.h"
#include "e_buff.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Initializes a CCITTT CRC lookup table.
 * Size parameter must be 256 and it's passed just for security. The
 * table can later be passed to e_crc_ccitt_t.
 * @param tab a table for CRC CCITT values.
 * @param the table size, MUST be 256.
 * @pre tab != NULL
 * @pre size == 256
 */
extern void e_crc_ccitt_init(u16 *tab, u16 size);

/** Calculate CCITT CRC using a lookup table.
 *
 *  If the table is NULL, no table will be used and the CRC will be slower.
 *
 *  @param crc the current CRC value, this will be updated with the result
 *  @param b the byte to update the CRC with
 *  @param tab the table initialized with e_crc_ccitt_init, NULL to
 *  not use a table.
 *  @pre crc != NULL
 *  @see e_crc_ccitt_init
 */
extern void e_crc_ccitt(u16 *crc, u8 b, u16 *tab);

/** Calculate CCITT CRC using a lookup table.
 *
 *  If the table is NULL, no table will be used and the CRC will be slower.
 *
 *  @param buf the byte buff
 *  @param size of buffer
 *  @param tab the table initialized with e_crc_ccitt_init, NULL to
 *  not use a table.
 *  @return the CRC value.
 *  @pre buff != NULL
 *  @see e_crc_ccitt_init
 */
extern u16 e_crc_ccitt_buff(u8* buff, u16 size, u16* tab);

/** Calculate CCITT CRC using a lookup table on an e_buff.
 *
 *  The e_buff state will be preserved.
 *
 *  If the table is NULL, no table will be used and the CRC will be slower.
 *
 *  @param crc where to store the CRC.
 *  @param buf the e_buff
 *  @param from first pos for CRC
 *  @param to last pos for CRC
 *  @param tab the table initialized with e_crc_ccitt_init, NULL to
 *  not use a table.
 *  @return the CRC value.
 *  @pre buff != NULL
 *  @pre from < e_buff_capacity(buff)
 *  @pre to < e_buff_capacity(buff)
 *  @pre from <= to
 *  @see e_crc_ccitt_init
 */
extern u16 e_crc_ccitt_ebuff(e_buff* buff, u32 from, u32 to, u16* tab);

#ifdef __cplusplus
}
#endif

#endif
