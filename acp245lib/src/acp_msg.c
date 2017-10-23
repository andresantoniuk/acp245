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

#include "acp_license.h"

#include "acp_msg_prov.c"
#include "acp_msg_conf.c"
#include "acp_msg_func.c"
#include "acp_msg_track.c"
#include "acp_msg_alarm.c"
#include "acp_msg_header.c"

/**
 * Simple Sanity check function to discard messages that have not been
 * initialized by the library.
 */
#define ACP_MSG_VALID(x)                    ((x).hdr.app_id != 0)

e_ret acp_msg_init(acp_msg *msg, acp_msg_app_id app_id, acp_msg_type type) {
    e_assert(msg != NULL);

    if (!acp_license_verified()) {
        return ERROR;
    }

    e_mem_set(msg, 0, sizeof(acp_msg));

    msg->hdr.app_id = app_id;
    msg->hdr.type = type;

    return OK;
}

void acp_msg_free(acp_msg* msg) {
    e_assert( msg != NULL );

    switch (msg->hdr.app_id) {
        case ACP_APP_ID_PROVISIONING:
            switch (msg->hdr.type) {
                case ACP_MSG_TYPE_PROV_UPD:
                    _free_msg_prov_upd(&msg->data.prov_upd); break;
                case ACP_MSG_TYPE_PROV_REPLY:
                    _free_msg_prov_reply(&msg->data.prov_reply); break;
                default:
                    E_DBG("Unknown message type on free: %lu",
                            (unsigned long) msg->hdr.type);
            }
            break;
        case ACP_APP_ID_CONFIGURATION:
            switch (msg->hdr.type) {
                case ACP_MSG_TYPE_CFG_UPD_245:
                    _free_msg_cfg_upd_245(&msg->data.cfg_upd_245); break;
                case ACP_MSG_TYPE_CFG_REPLY:
                    _free_msg_cfg_reply(&msg->data.cfg_reply); break;
                case ACP_MSG_TYPE_CFG_REPLY_245:
                    _free_msg_cfg_reply_245(&msg->data.cfg_reply_245); break;
	            case ACP_MSG_TYPE_CFG_ACT_245:
                    _free_msg_cfg_activation(&msg->data.cfg_activation); break;
                default:
                    E_DBG("Unknown message type on free: %lu",
                            (unsigned long) msg->hdr.type);
            }
            break;
        case ACP_APP_ID_REMOTE_VEHICLE_FUNCTION:
            switch (msg->hdr.type) {
                case ACP_MSG_TYPE_FUNC_CMD:
                    _free_msg_func_cmd(&msg->data.func_cmd); break;
                case ACP_MSG_TYPE_FUNC_STATUS:
                    _free_msg_func_status(&msg->data.func_status); break;
                default:
                    E_DBG("Unknown message type on free: %lu",
                            (unsigned long) msg->hdr.type);
            }
            break;
        case ACP_APP_ID_VEHICLE_TRACKING:
            switch (msg->hdr.type) {
                case ACP_MSG_TYPE_TRACK_CMD:
                    _free_msg_track_cmd(&msg->data.track_cmd); break;
                case ACP_MSG_TYPE_TRACK_POS:
                    _free_msg_track_pos(&msg->data.track_pos); break;
                case ACP_MSG_TYPE_TRACK_REPLY:
                    _free_msg_track_reply(&msg->data.track_reply); break;
                default:
                    E_DBG("Unknown message type on free: %lu",
                            (unsigned long) msg->hdr.type);
            }
            break;
        case ACP_APP_ID_ALARM:
            switch (msg->hdr.type) {
                case ACP_MSG_TYPE_ALARM_NOTIF:
                    _free_msg_alarm_notif(&msg->data.alarm_notif); break;
                case ACP_MSG_TYPE_ALARM_REPLY:
                    _free_msg_alarm_reply(&msg->data.alarm_reply); break;
                case ACP_MSG_TYPE_ALARM_POS:
                    _free_msg_alarm_pos(&msg->data.alarm_pos); break;
                case ACP_MSG_TYPE_ALARM_KA:
                    _free_msg_alarm_ka(&msg->data.alarm_ka); break;
                case ACP_MSG_TYPE_ALARM_KA_REPLY:
                    _free_msg_alarm_ka_reply(&msg->data.alarm_ka_reply); break;
                default:
                    E_DBG("Unknown message type on free: %lu",
                            (unsigned long) msg->hdr.type);
            }
            break;
        default:
            if (ACP_MSG_VALID(*msg)) {
                E_DBG("Unknown application id on free: %lu",
                        (unsigned long) msg->hdr.type);
            }
    }
}

e_ret acp_msg_write(e_buff *buff, acp_msg* msg) {
    e_ret rc;
    u32 start_lim;
    u32 hdr_lim;
    e_assert( buff != NULL );
    e_assert( msg != NULL );

    if (!acp_license_verified()) {
        return ERROR;
    }

    E_TRACE("2 Writing message app_id=%lu type=%lu buff_rem=%lu",
                (unsigned long) msg->hdr.app_id,
                (unsigned long) msg->hdr.type,
                (unsigned long) e_buff_write_remain(buff));

    start_lim = e_buff_get_lim(buff);

    /* write basic header, look below to see how its adapted
     * if message_length exceeds 0xFF bytes */
    if ((rc = _write_hdr(buff, &msg->hdr, 0))) {
        E_DBG("Error writing header: %x.", rc);
        goto exit;
    }

    hdr_lim = e_buff_get_lim(buff);

    rc = ACP_MSG_ERR_UNKNOWN_MSG_TYPE;
    switch (msg->hdr.app_id) {
        case ACP_APP_ID_PROVISIONING:
            switch (msg->hdr.type) {
                case ACP_MSG_TYPE_PROV_UPD:
                    rc = _write_msg_prov_upd(buff, &msg->data.prov_upd); break;
                case ACP_MSG_TYPE_PROV_REPLY:
                    rc = _write_msg_prov_reply(buff, &msg->data.prov_reply); break;
                case ACP_MSG_TYPE_PROV_UPD_COMMIT:
                case ACP_MSG_TYPE_PROV_REPLY_COMMIT:
                case ACP_MSG_TYPE_PROV_REQUEST:
                case ACP_MSG_TYPE_PROV_STATUS:
                    rc = ACP_MSG_ERR_UNSUP_MSG_TYPE; break;
                default:
                    rc = ACP_MSG_ERR_UNKNOWN_MSG_TYPE;
            }
            break;
        case ACP_APP_ID_CONFIGURATION:
            switch (msg->hdr.type) {
                case ACP_MSG_TYPE_CFG_UPD_245:
                    rc = _write_msg_cfg_upd_245(buff, &msg->data.cfg_upd_245); break;
                case ACP_MSG_TYPE_CFG_REPLY:
                    rc = _write_msg_cfg_reply(buff, &msg->data.cfg_reply); break;
                case ACP_MSG_TYPE_CFG_REPLY_245:
                    rc = _write_msg_cfg_reply_245(buff, &msg->data.cfg_reply_245); break;
	            case ACP_MSG_TYPE_CFG_ACT_245:
                    rc = _write_msg_cfg_activation(buff, &msg->data.cfg_activation); break;
	            case ACP_MSG_TYPE_CFG_UPD:
                case ACP_MSG_TYPE_CFG_UPD_COMMIT:
                case ACP_MSG_TYPE_CFG_REPLY_COMMIT:
                case ACP_MSG_TYPE_CFG_REQUEST:
                case ACP_MSG_TYPE_CFG_STATUS:
                case ACP_MSG_TYPE_CFG_EDIT:
                    rc = ACP_MSG_ERR_UNSUP_MSG_TYPE; break;
                default:
                    rc = ACP_MSG_ERR_UNKNOWN_MSG_TYPE;
            }
            break;
        case ACP_APP_ID_REMOTE_VEHICLE_FUNCTION:
            switch (msg->hdr.type) {
                case ACP_MSG_TYPE_FUNC_CMD:
                    rc = _write_msg_func_cmd(buff, &msg->data.func_cmd); break;
                case ACP_MSG_TYPE_FUNC_STATUS:
                    rc = _write_msg_func_status(buff, &msg->data.func_status); break;
	            case ACP_MSG_TYPE_FUNC_REQ:
                    rc = ACP_MSG_ERR_UNSUP_MSG_TYPE; break;
                default:
                    rc = ACP_MSG_ERR_UNKNOWN_MSG_TYPE;
            }
            break;
        case ACP_APP_ID_VEHICLE_TRACKING:
            switch (msg->hdr.type) {
                case ACP_MSG_TYPE_TRACK_CMD:
                    rc = _write_msg_track_cmd(buff, &msg->data.track_cmd); break;
                case ACP_MSG_TYPE_TRACK_POS:
                    rc = _write_msg_track_pos(buff, &msg->data.track_pos); break;
                case ACP_MSG_TYPE_TRACK_REPLY:
                    rc = _write_msg_track_reply(buff, &msg->data.track_reply); break;
                case ACP_MSG_TYPE_TRACK_WITH_COMMIT:
                case ACP_MSG_TYPE_TRACK_COMMIT:
                    rc = ACP_MSG_ERR_UNSUP_MSG_TYPE; break;
                default:
                    rc = ACP_MSG_ERR_UNKNOWN_MSG_TYPE;
            }
            break;
        case ACP_APP_ID_ALARM:
            switch (msg->hdr.type) {
                case ACP_MSG_TYPE_ALARM_NOTIF:
                    rc = _write_msg_alarm_notif(buff, &msg->data.alarm_notif); break;
                case ACP_MSG_TYPE_ALARM_REPLY:
                    rc = _write_msg_alarm_reply(buff, &msg->data.alarm_reply); break;
                case ACP_MSG_TYPE_ALARM_POS:
                    rc = _write_msg_alarm_pos(buff, &msg->data.alarm_pos); break;
                case ACP_MSG_TYPE_ALARM_KA:
                    rc = _write_msg_alarm_ka(buff, &msg->data.alarm_ka); break;
                case ACP_MSG_TYPE_ALARM_KA_REPLY:
                    rc = _write_msg_alarm_ka_reply(buff, &msg->data.alarm_ka_reply); break;
                default:
                    rc = ACP_MSG_ERR_UNKNOWN_MSG_TYPE;
            }
            break;
        default:
            rc = ACP_MSG_ERR_UNKNOWN_APP_ID;
    }

    /* The follwing code is required since the header length depends on
     * the body length, so we need to perform header mangling after
     * writing the body.
     *
     * Other options would be to precalculate message length (complex
     * and redundant), or to use intermediate buffers (not good for
     * memory constrained devices).
     */
    {
        u32 end_lim;
        u32 msg_len;
        u16 body_len;
        end_lim = e_buff_get_lim(buff);
        msg_len = end_lim - start_lim;

		/* valid ACP messages MUST BE less than 0xFFF */
        body_len = (u16) (end_lim - hdr_lim);

        if (msg_len >= 0xFFFF) {
            /* resulting message would exceed max msg length */
            rc = ACP_MSG_ERR_BAD_LENGTH;
            goto exit;
        } else if (msg_len > 0xFF) {
            /* perform header mangling to add an additional size byte
             * this is not cute nor performant, but it helps us avoid
             * using an intermediate buffer, which could be a problem
             * in embedded apps.
             * */

            /* move body one byte forward */
            if(e_buff_displace_fwd(buff, hdr_lim, 1)) {
                rc = ACP_MSG_ERR_INCOMPLETE;
                goto exit;
            }
            hdr_lim++; /* keep vars with the right info */
            end_lim++;

            /* replace previous header and use the byte gained from the
             * previous op*/
            e_buff_set_lim(buff, start_lim);
            if ((rc = _write_hdr(buff, &msg->hdr, body_len))) {
                E_DBG("Error writing header: %x.", rc);
                goto exit;
            }

            /* sanity checks for insane code... */
            e_assert(hdr_lim == e_buff_get_lim(buff));
            e_assert(start_lim + msg_len == end_lim);
            e_assert(hdr_lim - start_lim == msg_len - body_len);

            /* restore the buffer to the proper limit */
            e_buff_set_lim(buff, end_lim);
        } else {
            /* update message length */
            e_buff_set_lim(buff, hdr_lim -1);
            e_buff_write(buff, (u8) (msg_len & 0xFF));
            e_buff_set_lim(buff, end_lim);
        }
    }

exit:
    if (rc) {
        /* reset to original limit so written data is ignored */
        e_buff_set_lim(buff, start_lim);
    }
    E_DBG("Message written with result: %x.", rc);
    return rc;
}

e_ret acp_msg_read(e_buff *buff, acp_msg* msg) {
    e_ret rc;
    u32 start_pos;
    e_buff body;
    u16 body_len;

    e_assert( buff != NULL );
    e_assert( msg != NULL );

    if (!acp_license_verified()) {
        return ERROR;
    }

    start_pos = e_buff_get_pos(buff);

    acp_msg_init(msg, 0, 0);

    /* get message header */
    if ((rc = _read_hdr(buff, &msg->hdr, &body_len))) {
        E_DBG("Error reading header: %x.", rc);
        return rc;
    }

    /* get message body slice */
    if (e_buff_slice(buff, &body, body_len)) {
        E_DBG("Incomplete message.");
        return ACP_MSG_ERR_INCOMPLETE;
    }

    E_TRACE("Reading message app_id=%lu type=%lu body_len=%lu",
                (unsigned long) msg->hdr.app_id,
                (unsigned long) msg->hdr.type,
                (unsigned long) body_len);

    rc = ACP_MSG_ERR_UNKNOWN_MSG_TYPE;
    switch (msg->hdr.app_id) {
        case ACP_APP_ID_PROVISIONING:
            switch (msg->hdr.type) {
                case ACP_MSG_TYPE_PROV_UPD:
                    rc = _read_msg_prov_upd(&body, &msg->data.prov_upd); break;
                case ACP_MSG_TYPE_PROV_REPLY:
                    rc = _read_msg_prov_reply(&body, &msg->data.prov_reply); break;
                case ACP_MSG_TYPE_PROV_UPD_COMMIT:
                case ACP_MSG_TYPE_PROV_REPLY_COMMIT:
                case ACP_MSG_TYPE_PROV_REQUEST:
                case ACP_MSG_TYPE_PROV_STATUS:
                    rc = ACP_MSG_ERR_UNSUP_MSG_TYPE; break;
                default:
                    rc = ACP_MSG_ERR_UNKNOWN_MSG_TYPE;
            }
            break;
        case ACP_APP_ID_CONFIGURATION:
            switch (msg->hdr.type) {
                case ACP_MSG_TYPE_CFG_UPD_245:
                    rc = _read_msg_cfg_upd_245(&body, &msg->data.cfg_upd_245); break;
                case ACP_MSG_TYPE_CFG_REPLY:
                    rc = _read_msg_cfg_reply(&body, &msg->data.cfg_reply); break;
                case ACP_MSG_TYPE_CFG_REPLY_245:
                    rc = _read_msg_cfg_reply_245(&body, &msg->data.cfg_reply_245); break;
	            case ACP_MSG_TYPE_CFG_ACT_245:
                    rc = _read_msg_cfg_activation(&body, &msg->data.cfg_activation); break;
	            case ACP_MSG_TYPE_CFG_UPD:
                case ACP_MSG_TYPE_CFG_UPD_COMMIT:
                case ACP_MSG_TYPE_CFG_REPLY_COMMIT:
                case ACP_MSG_TYPE_CFG_REQUEST:
                case ACP_MSG_TYPE_CFG_STATUS:
                case ACP_MSG_TYPE_CFG_EDIT:
                    rc = ACP_MSG_ERR_UNSUP_MSG_TYPE; break;
                default:
                    rc = ACP_MSG_ERR_UNKNOWN_MSG_TYPE;
            }
            break;
        case ACP_APP_ID_REMOTE_VEHICLE_FUNCTION:
            switch (msg->hdr.type) {
                case ACP_MSG_TYPE_FUNC_CMD:
                    rc = _read_msg_func_cmd(&body, &msg->data.func_cmd); break;
                case ACP_MSG_TYPE_FUNC_STATUS:
                    rc = _read_msg_func_status(&body, &msg->data.func_status); break;
	            case ACP_MSG_TYPE_FUNC_REQ:
                    rc = ACP_MSG_ERR_UNSUP_MSG_TYPE; break;
                default:
                    rc = ACP_MSG_ERR_UNKNOWN_MSG_TYPE;
            }
            break;
        case ACP_APP_ID_VEHICLE_TRACKING:
            switch (msg->hdr.type) {
                case ACP_MSG_TYPE_TRACK_CMD:
                    rc = _read_msg_track_cmd(&body, &msg->data.track_cmd); break;
                case ACP_MSG_TYPE_TRACK_POS:
                    rc = _read_msg_track_pos(&body, &msg->data.track_pos); break;
                case ACP_MSG_TYPE_TRACK_REPLY:
                    rc = _read_msg_track_reply(&body, &msg->data.track_reply); break;
                case ACP_MSG_TYPE_TRACK_WITH_COMMIT:
                case ACP_MSG_TYPE_TRACK_COMMIT:
                    rc = ACP_MSG_ERR_UNSUP_MSG_TYPE; break;
                default:
                    rc = ACP_MSG_ERR_UNKNOWN_MSG_TYPE;
            }
            break;
        case ACP_APP_ID_ALARM:
            switch (msg->hdr.type) {
                case ACP_MSG_TYPE_ALARM_NOTIF:
                    rc = _read_msg_alarm_notif(&body, &msg->data.alarm_notif); break;
                case ACP_MSG_TYPE_ALARM_REPLY:
                    rc = _read_msg_alarm_reply(&body, &msg->data.alarm_reply); break;
                case ACP_MSG_TYPE_ALARM_POS:
                    rc = _read_msg_alarm_pos(&body, &msg->data.alarm_pos); break;
                case ACP_MSG_TYPE_ALARM_KA:
                    rc = _read_msg_alarm_ka(&body, &msg->data.alarm_ka); break;
                case ACP_MSG_TYPE_ALARM_KA_REPLY:
                    rc = _read_msg_alarm_ka_reply(&body, &msg->data.alarm_ka_reply); break;
                default:
                    rc = ACP_MSG_ERR_UNKNOWN_MSG_TYPE;
            }
            break;
        default:
            rc = ACP_MSG_ERR_UNKNOWN_APP_ID;
    }

    /* the entire body _was_ available, so if a body parsing function thought
     * that the message was incomplete it's because _some_ length field was
     * wrong */
    if (ACP_MSG_ERR_INCOMPLETE == rc)  {
        E_DBG("Msg read returned incomplete, changing to bad length.");
        rc = ACP_MSG_ERR_BAD_LENGTH;
    }

    E_DBG("Message parsed with result: %x.", rc);
    if (rc) {
        /* reset to original position so message can be re-read when
         * the buffer data is complete */
        e_buff_set_pos(buff, start_pos);
    }
    return rc;
}

e_ret acp_msg_read_data(u8* data, u32 data_len, u32 *readed, acp_msg *msg) {
    e_ret rc;
    e_buff msg_buff;

    if (!acp_license_verified()) {
        return ERROR;
    }

    e_buff_wrap(&msg_buff, data, data_len);
    e_buff_set_lim(&msg_buff, data_len);

    rc = acp_msg_read(&msg_buff, msg);

    if (readed) {
        *readed = data_len - e_buff_read_remain(&msg_buff);
    }

    e_buff_dealloc(&msg_buff);

    e_assert(!readed || *readed <= data_len);

    return rc;
}

e_ret acp_msg_write_data(u8* data, u32 data_len, u32 *written, acp_msg* msg) {
    e_ret rc;
    e_buff msg_buff;

    if (!acp_license_verified()) {
        return ERROR;
    }

    e_buff_wrap(&msg_buff, data, data_len);

    rc = acp_msg_write(&msg_buff, msg);
    if (written) {
        *written = e_buff_read_remain(&msg_buff);
    }
    e_buff_dealloc(&msg_buff);

    e_assert(!written || *written <= data_len);

    return rc;
}

bool acp_msg_is_reply_codes(acp_msg_app_id id, acp_msg_type type, acp_msg_app_id reply_id, acp_msg_type reply_type) {
    if (!acp_license_verified()) {
        return FALSE;
    }

    if (id != reply_id) {
        return FALSE;
    }

    switch(id) {
    case ACP_APP_ID_PROVISIONING:
        if (ACP_MSG_TYPE_PROV_UPD == type) {
            return ACP_MSG_TYPE_PROV_REPLY == reply_type;
        }
        break;
    case ACP_APP_ID_CONFIGURATION:
        if (ACP_MSG_TYPE_CFG_UPD_245 == type) {
            return ACP_MSG_TYPE_CFG_REPLY == reply_type ||
                ACP_MSG_TYPE_CFG_REPLY_245 == reply_type;
        }
        if (ACP_MSG_TYPE_CFG_ACT_245 == type) {
            return ACP_MSG_TYPE_CFG_REPLY == reply_type;
        }
        break;
    case ACP_APP_ID_REMOTE_VEHICLE_FUNCTION:
        if (ACP_MSG_TYPE_FUNC_CMD == type) {
            return ACP_MSG_TYPE_FUNC_STATUS == reply_type;
        }
        break;
    case ACP_APP_ID_VEHICLE_TRACKING:
        if (ACP_MSG_TYPE_TRACK_POS == type) {
            return ACP_MSG_TYPE_TRACK_REPLY == reply_type;
        }
        if (ACP_MSG_TYPE_TRACK_CMD == type) {
            return ACP_MSG_TYPE_TRACK_POS == reply_type;
        }
        break;
    case ACP_APP_ID_ALARM:
        if (ACP_MSG_TYPE_ALARM_NOTIF == type) {
            return ACP_MSG_TYPE_ALARM_REPLY == reply_type;
        }
        if (ACP_MSG_TYPE_ALARM_POS == type) {
            return ACP_MSG_TYPE_ALARM_REPLY == reply_type;
        }
        break;
    default:
        break;
    }
    return FALSE;
}

bool acp_msg_is_tcu_message(acp_msg_app_id id, acp_msg_type type) {
    if (!acp_license_verified()) {
        return FALSE;
    }
    return (
            (id == ACP_APP_ID_PROVISIONING && type == ACP_MSG_TYPE_PROV_REPLY) ||
            (id == ACP_APP_ID_CONFIGURATION &&
             (type == ACP_MSG_TYPE_CFG_REPLY ||
              type == ACP_MSG_TYPE_CFG_REPLY_245)) ||
            (id == ACP_APP_ID_REMOTE_VEHICLE_FUNCTION && type == ACP_MSG_TYPE_FUNC_STATUS) ||
            (id == ACP_APP_ID_VEHICLE_TRACKING && type == ACP_MSG_TYPE_TRACK_POS) ||
            (id == ACP_APP_ID_ALARM &&
             (type == ACP_MSG_TYPE_ALARM_NOTIF ||
              type == ACP_MSG_TYPE_ALARM_KA ||
              type == ACP_MSG_TYPE_ALARM_POS))
       );
}

bool acp_msg_is_so_message(acp_msg_app_id id, acp_msg_type type) {
    if (!acp_license_verified()) {
        return FALSE;
    }
    /* return !acp_msg_is_tcu_message would work, but would also reply true for unknown app_id/type */
    return (
            (id == ACP_APP_ID_PROVISIONING && type == ACP_MSG_TYPE_PROV_UPD) ||
            (id == ACP_APP_ID_CONFIGURATION &&
             (type == ACP_MSG_TYPE_CFG_UPD_245 ||
              type == ACP_MSG_TYPE_CFG_ACT_245)) ||
            (id == ACP_APP_ID_REMOTE_VEHICLE_FUNCTION && type == ACP_MSG_TYPE_FUNC_CMD) ||
            (id == ACP_APP_ID_VEHICLE_TRACKING && type == ACP_MSG_TYPE_TRACK_REPLY) ||
            (id == ACP_APP_ID_ALARM &&
             (type == ACP_MSG_TYPE_ALARM_REPLY ||
              type == ACP_MSG_TYPE_ALARM_KA_REPLY))
       );
}
