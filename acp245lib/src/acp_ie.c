/*=============================================================================
        Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

        This software is furnished under a license and may be used and copied
        only in accordance with the terms of such license and with the
        inclusion of the above copyright notice. This software or any other
        copies thereof may not be provided or otherwise made available to any
        other person. No title to and ownership of the software is hereby
        transferred.
==============================================================================*/
#include "acp245_config.h"

#include "acp_ie.h"
#include "acp_err.h"

#include "e_buff.h"
#include "e_log.h"
#include "e_mem.h"

#define IS_DIGIT(x) ((x >= 0x30) && (x <= 0x39))

u32 acp_ie_get_len(u32 val_len) {
    if (val_len > 0x0007C000) {
        return 4 + val_len;
    } else if (val_len > 0x00000F80) {
        return 3 + val_len;
    } else if (val_len > 0x0000001F) {
        return 2 + val_len;
    } else {
        return 1 + val_len;
    }
}

u32 acp_ie_get_iso_8859_1_len(ascii* text) {
    return acp_ie_get_len(text != NULL ? (u32) e_strlen(text) : 0);
}

u32 acp_ie_get_bcd_len(ascii* bcd) {
    return acp_ie_get_len(bcd != NULL ? ((((u32)e_strlen(bcd))+1)/2) : 0);
}

e_ret acp_ie_write_hdr(e_buff *buff, u8 type, u32 len) {
    e_ret rc;
    u8 b[4];
    u8 i;

    E_TRACE("writing IE hdr, len %lu type %x",
            (unsigned long) len,
            type);

    if (len > ACP_IE_MAX_LEN) {
        E_DBG("... not enough room in buff for message of len: %lu",
                (unsigned long) len);
        return ACP_MSG_ERR_TOO_LONG;
    }

    b[0] = ((type << 6 ) & 0xC0);
    if (len > 0x0007C000) {
        b[0] |=  0x20 | ((len >> 21) & 0x1F);
        b[1] = 0x80 | ((len >> 14) & 0x7F);
        b[2] = 0x80 | ((len >> 7) & 0x7F);
        b[3] = ((len) & 0x7F);
        i = 4;
    } else if (len > 0x00000F80) {
        b[0] |= 0x20 | ((len >> 14) & 0x1F);
        b[1] = 0x80 | ((len >> 7) & 0x7F);
        b[2] = ((len) & 0x7F);
        i = 3;
    } else if (len > 0x0000001F) {
        b[0] |= 0x20 | ((len >> 7) & 0x1F);
        b[1] = ((len) & 0x7F);
        i = 2;
    } else {
        b[0] |= (len & 0x1F);
        i = 1;
    }

    if (e_buff_write_remain(buff) < i + len) {
        E_DBG("Not enough data in buffer to write IE, needs=%lu.", (unsigned long) (i + len));
        return ACP_MSG_ERR_INCOMPLETE;
    }

    rc = e_buff_write_buff(buff, b, i);

    e_assert(rc == OK);

    return OK;
}

e_ret acp_ie_read(e_buff *buff, acp_ie *ie) {
    bool more;
    u16 len;
    u8 b1 = 0;

    e_assert( buff != NULL );
    e_assert( ie != NULL );

    if (e_buff_read(buff, &b1)) {
        return ACP_MSG_ERR_INCOMPLETE;
    }

    ie->id =  ((b1 & 0xC0) >> 6);
    more = ((b1 & 0x20) != 0);
    len = (b1 & 0x1F);

    while(more && (e_buff_read_remain(buff) > 0)) {
        u8 b_len;

        if (len & (0x7f << 9)) {
            /* an additional read would overflow len */
            return ACP_MSG_ERR_TOO_LONG;
        }

        if (e_buff_read(buff, &b_len)) {
            return ACP_MSG_ERR_INCOMPLETE;
        }

        more = (b_len & 0x80) != 0;
        len = (len << 7) | (b_len & 0x7F);
    }

    if (more) {
        return ACP_MSG_ERR_INCOMPLETE;
    }

    ie->len = len;
    if (e_buff_slice(buff, &ie->data, ie->len)) {
        E_DBG("Not enough data in buffer, expected %lu got %lu.",
            (unsigned long) len,
            (unsigned long) e_buff_read_remain(buff));
        return ACP_MSG_ERR_INCOMPLETE;
    }

    e_assert (IE_LEN(*ie) == ie->len);
    e_assert ((IE_LEN(*ie) > 0) || !IE_EXIST(*ie));
    e_assert (IE_REMAIN(*ie) == IE_LEN(*ie));

    return OK;
}

e_ret acp_ie_read_exp(e_buff *buff, acp_ie *ie, u8 id_exp) {
    e_ret rc;

    e_assert( buff != NULL );
    e_assert( ie != NULL );

    rc = acp_ie_read(buff, ie);
    if ((OK == rc) && (id_exp != ie->id)) {
        E_DBG("Unexpected IE id: expected=%lu got=%lu",
            (long unsigned) id_exp,
            (long unsigned) ie->id);
        rc = ACP_MSG_ERR_INVALID_DEFAULT;
    }
    return rc;
}

e_ret acp_ie_read_exp_l(e_buff *buff, acp_ie *ie, u8 id_exp, u32 len_exp) {
    e_ret rc;

    e_assert( buff != NULL );
    e_assert( ie != NULL );

    rc = acp_ie_read_exp(buff, ie, id_exp);
    if (OK == rc) {
        if (len_exp != IE_REMAIN(*ie)) {
            E_DBG("Unexpected IE len: expected=%lu got=%lu",
                (long unsigned) len_exp,
                (long unsigned) (IE_REMAIN(*ie)));
            rc = ACP_MSG_ERR_INVALID_DEFAULT;
        }
    }
    return rc;
}

e_ret acp_ie_read_byte(e_buff *buff, u8 *b) {
    e_ret rc;
    acp_ie ie;

    e_assert( buff != NULL );

    if ((rc = acp_ie_read_exp_l(buff, &ie, ACP_IE_BINARY, 1)) == OK) {
        e_buff_read(&ie.data, b);
    }

    return rc;
}

e_ret acp_ie_write_byte(e_buff *buff, u8 b) {
    e_ret rc;

    e_assert( buff != NULL );

    rc = acp_ie_write_hdr(buff, ACP_IE_BINARY, 1);

    if (OK == rc) {
        rc = e_buff_write(buff, b);
    }

    E_TRACE("acp_ie_write_byte exit, %x.", b);

    return OK;
}

e_ret acp_ie_read_bin_payload(acp_ie *ie, u8** data, u32 data_len) {
    e_assert( ie != NULL );
    e_assert( data != NULL );
    /* Try to read it as binary, even if id doesn't match */
    /*e_assert( ie->id == ACP_IE_BINARY || ie->id == ACP_IE_EXT_BINARY );*/

    E_TRACE("_read_bin enter, len=%ld", (unsigned long) data_len);

    if (IE_REMAIN(*ie) < data_len) {
        return ACP_MSG_ERR_INCOMPLETE;
    }

    if (0 == data_len) {
        E_DBG("IE with no data.");
        *data = NULL;
        return OK;
    }

    *data = e_mem_malloc(sizeof(u8) * (data_len));
    if (NULL == *data) {
        return ACP_MSG_ERR_NO_MEM;
    }

    e_buff_read_buff(&ie->data, *data, data_len);
    return OK;
}

e_ret acp_ie_read_bin(e_buff *buff, u8** data, u32* data_len) {
    e_ret rc;
    acp_ie ie;

    e_assert( buff != NULL );
    e_assert( data != NULL );

    if ((rc = acp_ie_read_exp(buff, &ie, ACP_IE_BINARY))) {
        return rc;
    }

    *data_len = IE_REMAIN(ie);
    return acp_ie_read_bin_payload(&ie, data, *data_len);
}

e_ret acp_ie_put_bin(e_buff *buff, u8* data, u32 data_len) {
    e_ret rc;
    acp_ie ie;

    e_assert( buff != NULL );
    e_assert( data != NULL );

    if ((rc = acp_ie_read_exp_l(buff, &ie, ACP_IE_BINARY, data_len))) {
        return rc;
    }

    rc = e_buff_read_buff(&ie.data, data, data_len);

    e_assert (rc == OK);

    return OK;
}

e_ret acp_ie_write_bin(e_buff *buff, u8* data, u32 len) {
    e_ret rc;

    e_assert( buff != NULL );
    e_assert( data != NULL || len == 0);

    if ((rc = acp_ie_write_hdr(buff, ACP_IE_BINARY, len))) {
        return rc;
    }

    if (data != NULL) {
        if (e_buff_write_buff(buff, data, len)) {
            return ACP_MSG_ERR_INCOMPLETE;
        }
    }

    return OK;
}

e_ret acp_ie_read_iso_8859_1_payload(acp_ie *ie, ascii** data) {
    u32 len;

    e_assert( ie != NULL );
    e_assert( data != NULL );
    e_assert( ie->id == ACP_IE_ISO_8859_1 || ie->id == ACP_IE_EXT_ISO_8859_1 );

    len = IE_REMAIN(*ie);
    E_TRACE("_read_iso_8859_1 enter, len=%ld", (unsigned long) len);

    if (0 == len) {
        E_DBG("IE with no data.");
        *data = NULL;
        return OK;
    }

    *data = e_mem_malloc(sizeof(ascii) * (len + 1));
    if (NULL == *data) {
        return ACP_MSG_ERR_NO_MEM;
    }

    e_buff_read_asciibuff(&ie->data, *data, len);
    (*data)[len] = '\0';
    return OK;
}

e_ret acp_ie_read_iso_8859_1(e_buff *buff, ascii** data) {
    e_ret rc;
    acp_ie ie;

    e_assert( buff != NULL );
    e_assert( data != NULL );

    if ((rc = acp_ie_read_exp(buff, &ie, ACP_IE_ISO_8859_1))) {
        return rc;
    }

    return acp_ie_read_iso_8859_1_payload(&ie, data);
}

e_ret acp_ie_write_iso_8859_1(e_buff *buff, ascii* data) {
    e_ret rc;
    u32 len;

    len = data != NULL ? e_strlen(data) : 0;

    rc = acp_ie_write_hdr(buff, ACP_IE_ISO_8859_1, len);
    if ((OK == rc) && (data != NULL)) {
        rc = e_buff_write_asciibuff(buff, data, len);
    }

    E_TRACE("acp_ie_write_iso_8859_1 exit, len=%lu.", (unsigned long) len);
    return OK;
}

e_ret acp_ie_read_bcd_payload(acp_ie *ie, ascii **data) {
    u32 len;
    u32 i;

    e_assert( ie != NULL );
    e_assert( data != NULL );
    e_assert( ie->id == ACP_IE_PACKED_DEC || ie->id == ACP_IE_EXT_PACKED_DEC );

    len = IE_REMAIN(*ie);
    if (0 == len) {
        *data = NULL;
    }

    *data = e_mem_malloc(sizeof(ascii) * (len*2) + 1);
    if (NULL == *data) {
        return ACP_MSG_ERR_NO_MEM;
    }

    for(i = 0; i < len; i++) {
        u8 b;
        u32 c;
        u8 d1;
        u8 d2;

        e_buff_read(&ie->data, &b);
        d1 = ((b&0xF0)>>4);
        d2 = (b&0x0F);
        if (d1 > 9 || d2 > 9) {
            e_mem_free(*data);
            *data = NULL;
            E_DBG("acp_ie_read_bcd_payload bad format, not a BCD value");
            return ACP_MSG_ERR_BAD_FORMAT;
        }

        c = i*2;
        (*data)[c] = (ascii) (0x30 + d1);
        (*data)[c+1] = (ascii) (0x30 + d2);

    }
    (*data)[len*2] = '\0';

    return OK;
}

e_ret acp_ie_read_bcd(e_buff *buff, ascii** data) {
    e_ret rc;
    acp_ie ie;

    e_assert( buff != NULL );
    e_assert( data != NULL );

    if ((rc = acp_ie_read_exp(buff, &ie, ACP_IE_PACKED_DEC))) {
        return rc;
    }

    return acp_ie_read_bcd_payload(&ie, data);
}

e_ret acp_ie_write_bcd(e_buff *buff, ascii* data) {
    e_ret rc = OK;
    u8 b = 0;
    u32 len;

    if (data == NULL) {
        goto exit;
    }

    len = e_strlen(data);

    if (0 == len
        || ((rc = acp_ie_write_hdr(buff, ACP_IE_PACKED_DEC, ((len+1)/ 2))))) {
        goto exit;
    }

    if (len % 2 != 0) {
        if (!IS_DIGIT(*data)) {
            E_DBG("acp_ie_write_bcd bad format, not a BCD value");
            rc = ACP_MSG_ERR_BAD_FORMAT;
            goto exit;
        }
        e_buff_write(buff, (*data++ & 0xF));
    }

    while (*data != '\0') {

        if (!IS_DIGIT(*data)) {
            E_DBG("acp_ie_write_bcd bad format, not a BCD value");
            rc = ACP_MSG_ERR_BAD_FORMAT;
            goto exit;
        }
        /* *data is positive since its in the printable integer ascii range  */
        b = (*data++ & 0xF) << 4;

        if (!IS_DIGIT(*data)) {
            E_DBG("acp_ie_write_bcd bad format, not a BCD value");
            rc = ACP_MSG_ERR_BAD_FORMAT;
            goto exit;
        }
        b |= (*data++ & 0xF);

        e_buff_write(buff, b);
    }

exit:
    E_TRACE("acp_ie_write_bcd exit");

    return rc;
}

void acp_ie_free_any(acp_ie_any *ie) {
    switch(ie->id) {
        case ACP_IE_ISO_8859_1:
        case ACP_IE_PACKED_DEC:
            e_mem_free(ie->data.str);
            ie->data.str = NULL;
            break;
        case ACP_IE_BINARY:
        default:
            e_mem_free(ie->data.bin);
            ie->data.bin = NULL;
            break;
    }
}

u32 acp_ie_get_any_len(acp_ie_any *ie) {
    if (!ie->present) {
        return 0;
    } else if (ACP_IE_BINARY == ie->id) {
        return acp_ie_get_len(ie->len);
    } else if (ACP_IE_ISO_8859_1 == ie->id) {
        return acp_ie_get_iso_8859_1_len(ie->data.str);
    } else if (ACP_IE_PACKED_DEC == ie->id) {
        return acp_ie_get_bcd_len(ie->data.str);
    } else {
        return acp_ie_get_len(ie->len);
    }
}

e_ret acp_ie_read_any(e_buff *buff, acp_ie_any *ie_any) {
    e_ret rc;
    acp_ie ie;

    if (e_buff_read_remain(buff) == 0) {
        ie_any->present = FALSE;
        rc = OK;
        goto exit;
    }

    if ((rc = acp_ie_read(buff, &ie))) {
        goto exit;
    }

    ie_any->present = TRUE;

    if (IE_EXIST(ie)) {
        ie_any->id = ie.id;
        if (ACP_IE_BINARY == ie.id) {
            ie_any->len = IE_REMAIN(ie);
            if ((rc = acp_ie_read_bin_payload(&ie, &ie_any->data.bin, ie_any->len))) {
                goto exit;
            }
        } else if (ACP_IE_ISO_8859_1 == ie.id) {
            if ((rc = acp_ie_read_iso_8859_1_payload(&ie, &ie_any->data.str))) {
                ie_any->data.str = NULL;
                goto exit;
            }
        } else if (ACP_IE_PACKED_DEC == ie.id) {
            if ((rc = acp_ie_read_bcd_payload(&ie, &ie_any->data.str))) {
                goto exit;
            }
        } else {
            ie_any->len = IE_REMAIN(ie);
            if ((rc = acp_ie_read_bin_payload(&ie, &ie_any->data.bin, ie_any->len))) {
                goto exit;
            }
        }
    }

    rc = OK;

exit:
    return rc;
}

e_ret acp_ie_write_any(e_buff *buff, acp_ie_any *ie) {
    e_ret rc;

    if (ACP_IE_BINARY == ie->id) {
        if ((rc = acp_ie_write_bin(buff, ie->data.bin, ie->len))) {
            goto exit;
        }
    } else if (ACP_IE_ISO_8859_1 == ie->id) {
        if ((rc = acp_ie_write_iso_8859_1(buff, ie->data.str))) {
            goto exit;
        }
    } else if (ACP_IE_PACKED_DEC == ie->id) {
        if ((rc = acp_ie_write_bcd(buff, ie->data.str))) {
            goto exit;
        }
    } else {
        if ((rc = acp_ie_write_bin(buff, ie->data.bin, ie->len))) {
            goto exit;
        }
    }
    rc = OK;

exit:
    return rc;
}
