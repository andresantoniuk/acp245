/*=============================================================================
  Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

  This software is furnished under a license and may be used and copied
  only in accordance with the terms of such license and with the
  inclusion of the above copyright notice. This software or any other
  copies thereof may not be provided or otherwise made available to any
  other person. No title to and ownership of the software is hereby
  transferred.
  ==============================================================================*/
/*=============================================================================
 *    Description:  ACP 245 Activation Key Verifier.
 *        Created:  07/28/2009 01:39:16 PM
 *         Author:  Edantech
 *   Contributors:  Santiago Aguiar, santiago.aguiar@edantech.com
 ==============================================================================*/
#include "acp245_config.h"

#include "acp_key.h"

#include "e_port.h"
#include "e_mem.h"
#include "e_log.h"
#include "e_util.h"
#include "hmac_sha256.h"
#include "sha256.h"

static u8 _msg_buff[ACP_KEY_MAX_MSG_LEN];

e_ret acp_key_get(
        u8 *kt, u8 kt_len,
        u8 *iccid, u8 iccid_len,
        u8 *date, u8 date_len,
        u8 *msg, u16 msg_len,
        u8 *ks, u8 ks_len) {
    u8 kd[SHA256_HASH_SIZE];

    e_assert(kt != NULL);
    e_assert(iccid != NULL);
    e_assert(date != NULL);
    e_assert(msg != NULL);
    e_assert(ks != NULL);

    if (kt_len > ACP_KEY_MAX_KT_LEN) {
        return ACP_KEY_ERR_INVALID_PARAM_LEN;
    }

    if (ks_len > ACP_KEY_MAX_KS_LEN) {
        return ACP_KEY_ERR_INVALID_PARAM_LEN;
    }

    if (iccid_len + date_len > ACP_KEY_MAX_MSG_LEN) {
        E_DBG("iccid + date_len to big: %lu",
                (unsigned long) (iccid_len + date_len));
        return ACP_KEY_ERR_INVALID_PARAM_LEN;
    }

    (void) e_mem_cpy(_msg_buff, iccid, iccid_len);
    (void) e_mem_cpy(_msg_buff + iccid_len, date, date_len);

    hmac_sha256(kt, kt_len, _msg_buff, iccid_len + date_len,
            kd, SHA256_HASH_SIZE);

    if (E_LOG_IS(DEBUG)) {
        ascii *hex_key;
        ascii *hex_msg;
        ascii *hex_mac;
        /* TODO: REMOVE THE FOLLOWING LINE */
        hex_key = e_util_to_hex(kt, kt_len);
        hex_msg = e_util_to_hex(_msg_buff, iccid_len + date_len);
        hex_mac = e_util_to_hex(kd, SHA256_HASH_SIZE);
        E_DBG("kt='%s' iccid+date='%s' mac='%s'", hex_key, hex_msg, hex_mac);
        e_mem_free(hex_key);
        e_mem_free(hex_msg);
        e_mem_free(hex_mac);
    }

    hmac_sha256(kd, SHA256_HASH_SIZE, msg, msg_len, ks, ks_len);

    if (E_LOG_IS(DEBUG)) {
        ascii *hex_key;
        ascii *hex_msg;
        ascii *hex_mac;
        /* TODO: REMOVE THE FOLLOWING LINE */
        hex_key = e_util_to_hex(kd, SHA256_HASH_SIZE);
        hex_msg = e_util_to_hex(msg, msg_len);
        hex_mac = e_util_to_hex(ks, ks_len);
        E_DBG("kd='%s' msg='%s' mac='%s'", hex_key, hex_msg, hex_mac);
        e_mem_free(hex_key);
        e_mem_free(hex_msg);
        e_mem_free(hex_mac);
    }

    return OK;
}

e_ret acp_key_verify(
        u8 *kt, u8 kt_len,
        u8 *ks, u8 ks_len,
        u8 *iccid, u8 iccid_len,
        u8 *date, u8 date_len,
        u8 *msg, u16 msg_len) {
    e_ret rc;
    u8 auth_key[ACP_KEY_AUTH_KEY_LEN];

    e_assert(kt != NULL);
    e_assert(iccid != NULL);
    e_assert(date != NULL);
    e_assert(msg != NULL);

    if (ks == NULL || ks_len == 0) {
        return ACP_KEY_ERR_NO_KEY;
    }

    if (ks_len < ACP_KEY_AUTH_KEY_LEN) {
        E_DBG("authentication key to small: %lu",
                (unsigned long) ks_len);
        return ACP_KEY_ERR_INVALID_AUTH_KEY_LEN;
    }

    rc = acp_key_get(kt, kt_len,
            iccid, iccid_len,
            date, date_len,
            msg, msg_len,
            auth_key, ACP_KEY_AUTH_KEY_LEN);
    if (rc) {
        return rc;
    }

    if (e_mem_cmp(ks, auth_key, ACP_KEY_AUTH_KEY_LEN) == 0) {
        return OK;
    } else {
        if (E_LOG_IS(DEBUG)) {
            ascii *hex_expected;
            ascii *hex_got;
            hex_expected = e_util_to_hex(ks, ks_len);
            hex_got = e_util_to_hex(auth_key, ACP_KEY_AUTH_KEY_LEN);
            E_DBG("Bad key. Expected '%s' got '%s'",
                    hex_expected,
                    hex_got
                    );
            e_mem_free(hex_expected);
            e_mem_free(hex_got);
        }

        return ACP_KEY_ERR_BAD_KEY;
    }
}

e_ret acp_key_verify_msg(
        u8 *kt, u8 kt_len,
        u8 *iccid, u8 iccid_len,
        u8 *date, u8 date_len,
        acp_msg *msg) {
    e_ret rc;
    u8 buff_data[256];
    u8 auth_key_cpy[256];
    e_buff buff;
    u8 *auth_key;
    u8 auth_key_len;

    e_assert(kt != NULL);
    e_assert(iccid != NULL);
    e_assert(date != NULL);
    e_assert(msg != NULL);

    if (ACP_APP_ID_CONFIGURATION != msg->hdr.app_id
            || ACP_MSG_TYPE_CFG_ACT_245 != msg->hdr.type) {
        return ACP_KEY_ERR_INVALID_MSG;
    }

    auth_key = msg->data.cfg_activation.vehicle_desc.auth_key;
    auth_key_len = msg->data.cfg_activation.vehicle_desc.auth_key_len;
    if (auth_key == NULL || auth_key_len == 0) {
        return ACP_KEY_ERR_NO_KEY;
    }

    e_mem_cpy(auth_key_cpy, auth_key, auth_key_len);

    /* zero auth-key before writing message */
    e_mem_set(auth_key, 0, auth_key_len);
    e_buff_wrap(&buff, buff_data, 256);

    rc = acp_msg_write(&buff, msg);

    /* restore auth-key */
    e_mem_cpy(auth_key, auth_key_cpy, auth_key_len);

    if (rc) {
        return ACP_KEY_ERR_INVALID_MSG;
    }

    return acp_key_verify(
            kt, kt_len,
            auth_key, auth_key_len,
            iccid, iccid_len,
            date, date_len,
            buff_data, (u16) e_buff_read_remain(&buff)
    );
}

e_ret acp_key_get_msg(
        u8 *kt, u8 kt_len,
        u8 *iccid, u8 iccid_len,
        u8 *date, u8 date_len,
        acp_msg *msg) {
    e_ret rc;
    u8 buff_data[256];
    e_buff buff;
    u8 *old_auth_key = NULL;
    u8 old_auth_key_len;
    u8 *new_auth_key = NULL;
    u8 new_auth_key_len;

    e_assert(kt != NULL);
    e_assert(iccid != NULL);
    e_assert(date != NULL);
    e_assert(msg != NULL);

    if (ACP_APP_ID_CONFIGURATION != msg->hdr.app_id
            || ACP_MSG_TYPE_CFG_ACT_245 != msg->hdr.type) {
        return ACP_KEY_ERR_INVALID_MSG;
    }

    new_auth_key_len = ACP_KEY_AUTH_KEY_LEN;
    new_auth_key = e_mem_malloc(new_auth_key_len);
    if (!new_auth_key) {
        return ERROR;
    }
    e_mem_set(new_auth_key, 0, new_auth_key_len);

    old_auth_key = msg->data.cfg_activation.vehicle_desc.auth_key;
    old_auth_key_len = msg->data.cfg_activation.vehicle_desc.auth_key_len;

    msg->data.cfg_activation.vehicle_desc.auth_key = new_auth_key;
    msg->data.cfg_activation.vehicle_desc.auth_key_len = new_auth_key_len;

    e_buff_wrap(&buff, buff_data, 256);
    rc = acp_msg_write(&buff, msg);
    if (rc) {
        rc = ACP_KEY_ERR_INVALID_MSG;
        goto exit;
    }

    rc = acp_key_get(
            kt, kt_len,
            iccid, iccid_len,
            date, date_len,
            buff_data, (u16) e_buff_read_remain(&buff),
            new_auth_key, new_auth_key_len);
exit:
    if (rc) {
        e_mem_free(new_auth_key);
        msg->data.cfg_activation.vehicle_desc.auth_key = old_auth_key;
        msg->data.cfg_activation.vehicle_desc.auth_key_len = old_auth_key_len;
    } else {
        e_mem_free(old_auth_key);
    }
    return rc;
}
