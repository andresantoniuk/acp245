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

#include "acp_msg.h"

#include "e_port.h"
#include "e_log.h"
#include "e_mem.h"
#include "e_buff.h"

#include "acp_err.h"
#include "acp_el.h"

static e_ret _read_hdr(e_buff *buff, acp_hdr* hdr, u16 *body_len){
    e_ret rc = OK;
    u8 d[3];
    u8 b;
    u16 msg_len;
    u8 hdr_len;

    e_assert( buff != NULL );
    e_assert( hdr != NULL );

    if (e_buff_read_buff(buff, d, 3)) {
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }

    /* bit 0..1 reserved */
    hdr->app_id = d[0] & 0x3F;

    /* bit 0..1 reserved */
    hdr->test = ((d[1] & 0x30) != 0);
    hdr->type = d[1] & 0x1F;
    hdr->version = (d[2] & 0x70) >> 4;
    hdr->msg_ctrl = d[2] & 0x0F;
    hdr_len = 3;

    /* check version flag */
    if (d[2] & 0x80) {
        if (e_buff_read(buff, &b)) {
            rc = ACP_MSG_ERR_INCOMPLETE;
            goto exit;
        }

        if (7u == hdr->version) {
            /* bit 1..5 is extended version element */
            hdr->version = (b & 0x7C) >> 2;
            if (hdr->version < 7u) {
                /* it doesn't make sense, but the field has a minimum of 7 */
                E_DBG("Bad format, extended version field can't be less than 7");
                rc = ACP_MSG_ERR_BAD_FORMAT;
                goto exit;
            }
        } /* else bit 1..5 reserved */

        hdr->msg_prio = (b & 0x3);
        hdr_len += 1;

        /* check more flag and skip additional flags */
        while (b & ACP_MORE_FLG) {
            if (e_buff_read(buff, &b)) {
                rc = ACP_MSG_ERR_INCOMPLETE;
                goto exit;
            }
        }
    } else {
        hdr->msg_prio = 0;
    }

    if (e_buff_read(buff, &b)) {
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }

    if (hdr->msg_ctrl & ACP_HDR_MSG_CTRL_16BIT_LEN) {
        msg_len = (b << 8);
        if (e_buff_read(buff, &b)) {
            rc = ACP_MSG_ERR_INCOMPLETE;
            goto exit;
        }
        msg_len |= b;
        hdr_len += 2;
    } else {
        msg_len = b;
        hdr_len += 1;
    }

    if (hdr_len > msg_len) {
        E_DBG("Bad format, invalid message length");
        rc = ACP_MSG_ERR_BAD_FORMAT;
        goto exit;
    }

    *body_len = msg_len - hdr_len;

    E_DBG("Readed header, hdr_len=%lu, body_len=%lu, "
          "buff_len=%lu, msg_ctrl=%lu, version=%lu, "
          "msg_prio=%lu.",
            (unsigned long) hdr_len,
            (unsigned long) *body_len,
            (unsigned long) e_buff_read_remain(buff),
            (unsigned long) hdr->msg_ctrl,
            (unsigned long) hdr->version,
            (unsigned long) hdr->msg_prio
    );
exit:
    if(rc) {
        E_DBG("Invalid header: %lx", (long unsigned) rc);
    }

	return rc;
}

static e_ret _write_hdr(e_buff *buff, acp_hdr* hdr, u16 body_len) {
    e_ret rc = OK;
    u32 msg_len;
    bool ext_version;
    bool ext_header;
    e_assert( buff != NULL );
    e_assert( hdr != NULL );

    E_TRACE("_write_hdr enter, len=%lu.", (unsigned long) e_buff_write_remain(buff));

    /* CAUTION: msg_len must match written field lengths! */
    msg_len = body_len
        + 1 /* app_id  */
        + 1 /* type */
        + 1 /* version */
        + 1 /* min 1 byte for len */;

    /* 4.1.11, if version > 6, the extended version field must be used. */
    ext_version = (hdr->version > 6u);
    ext_header = (hdr->msg_prio != 0 || ext_version);
    if (ext_header) {
        msg_len++;
    }
    if (msg_len > 0xFF) {
        /* an additional length byte */
        msg_len++;
    }
    if (msg_len > 0xFFFF) {
        /* protocol does not allows messages longer than 0xFFFF */
        rc = ACP_MSG_ERR_BAD_LENGTH;
        goto exit;
    }

    if (e_buff_write(buff,
                /*bit 0..1 reserved*/
                hdr->app_id & 0x3F)) {
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }

    if (e_buff_write(buff,
                /*bit 0..1 reserved*/
                ((hdr->test << 5) & 0x30) |
                (hdr->type & 0x1F)
                )) {
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }

    if (e_buff_write(buff,
                (ext_header ? 0x80 : 0x0) |   /* version flag = 1 | 0 */
                (ext_version ? 0x70 : ((hdr->version & 0x07) << 4)) |
                (((msg_len > 0xFF ? ACP_HDR_MSG_CTRL_16BIT_LEN : 0) |
                  (hdr->msg_ctrl & ACP_HDR_MSG_CTRL_RESP_EXP))  & 0x0F)
                )) {
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }

    if (ext_header &&
            e_buff_write(buff,
                /* more flag = 0 | */
                (ext_version ? (hdr->version & 0x1F) << 2 : 0x00) |
                (hdr->msg_prio & 0x03)
                )) {
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }

    if (msg_len > 0xFF) {
        if (e_buff_write_u16(buff, (u16) msg_len)) {
            rc = ACP_MSG_ERR_INCOMPLETE;
            goto exit;
        }
    } else {
        if(e_buff_write(buff, (u8) msg_len)) {
            rc = ACP_MSG_ERR_INCOMPLETE;
            goto exit;
        }
    }

exit:
    E_TRACE("_write_hdr exit.");
    return rc;
}
