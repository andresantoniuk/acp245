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

#include "e_check.h"
#include "e_buff.h"

START_TEST (test_e_crc_ccitt)
{
    u16 tab[256];
    u16 crc;

    crc = 0;
    e_crc_ccitt(&crc, 0x0F, NULL);
    ARE_EQ_INT(0xF1EF, crc);

    e_crc_ccitt(&crc, 0xAB, NULL);
    ARE_EQ_INT(0x14BF, crc);

    e_crc_ccitt_init(tab, 256);
    e_crc_ccitt(&crc, 0x55, tab);
    ARE_EQ_INT(0xE7E5, crc);
}
END_TEST

START_TEST (test_e_crc_ccitt_ebuff)
{
    u16 tab[256];
    u8 buff[256];
    e_buff eb;
    u16 crc;
    int i;

    e_buff_wrap(&eb, buff, 256);

    for(i = 0; i < 256; i++) {
        e_buff_write(&eb, (u8) (i&0xFF));
    }

    crc = e_crc_ccitt_ebuff(&eb, 0, 256, NULL);
    ARE_EQ_INT(0x7E55, crc);

    e_crc_ccitt_init(tab, 256);
    crc = e_crc_ccitt_ebuff(&eb, 0, 256, tab);
    ARE_EQ_INT(0x7E55, crc);
}
END_TEST

START_TEST (test_e_crc_ccitt_buff)
{
    u16 tab[256];
    u8 buff[256];
    u16 crc;
    int i;

    for(i = 0; i < 256; i++) {
        buff[i] = i;
    }

    crc = e_crc_ccitt_buff(buff, 256, NULL);
    ARE_EQ_INT(0x7E55, crc);

    e_crc_ccitt_init(tab, 256);
    crc = e_crc_ccitt_buff(buff, 256, tab);
    ARE_EQ_INT(0x7E55, crc);
}
END_TEST

extern int e_crc_test (void);
int e_crc_test (void) {
    return e_check_run_suite("e_crc", 
            test_e_crc_ccitt, 
            test_e_crc_ccitt_buff, 
            test_e_crc_ccitt_ebuff, 
            NULL);
}
#ifndef USE_SINGLE_TEST
int main (void) {
    return e_crc_test();
}
#endif
