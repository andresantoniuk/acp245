/*=============================================================================
  Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

  This software is furnished under a license and may be used and copied
  only in accordance with the terms of such license and with the
  inclusion of the above copyright notice. This software or any other
  copies thereof may not be provided or otherwise made available to any
  other person. No title to and ownership of the software is hereby
  transferred.
  ==============================================================================*/
/**
 * ACP 245 activation key verifier functions.
 *
 * This file provides a set of functions to generate and verify an ACP 245
 * activation key.
 *
 * @file acp_key.h
 * @date  03/13/2009 01:51:28 PM
 * @author Edantech
 */
/*
 * Contributors:  Santiago Aguiar, santiago.aguiar@edantech.com
 */
#ifndef __acp_key_h__
#define __acp_key_h__

/** @cond EXPORT (doxygen cond) */

/* Definitions for DLL exports for WIN32 platforms. */
#if defined _MSC_VER || defined _WIN32
#ifdef E_ACP245_EXPORT_DLL
#define E_EXPORT __declspec (dllexport) extern
#else
#define E_EXPORT __declspec (dllimport) extern
#endif /* E_EXPORT_DLL */
#else
#define E_EXPORT extern
#endif /* _MSC_VER */

/** @endcond */

#include "e_port.h"
#include "acp_msg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ACP_KEY_MAX_KT_LEN      (20)
#define ACP_KEY_MAX_KS_LEN      (32)
#define ACP_KEY_MAX_MSG_LEN     (32)
#define ACP_KEY_AUTH_KEY_LEN    (8)

#define ACP_KEY_ERR_INVALID_AUTH_KEY_LEN    ((e_ret)0x8001)
#define ACP_KEY_ERR_INVALID_PARAM_LEN       ((e_ret)0x8002)
#define ACP_KEY_ERR_BAD_KEY                 ((e_ret)0x8003)
#define ACP_KEY_ERR_NO_KEY                  ((e_ret)0x8004)
#define ACP_KEY_ERR_INVALID_MSG             ((e_ret)0x8005)

/**
 * Verifies if a key is valid for a byte array.
 *
 * @param kt the TCU secret key (Kt)
 * @param kt_len the length of the TCU key
 * @param ks the authentication key.
 * @param ks_len the length of the authentication key.
 * @param iccid the ICCID of the TCU
 * @param iccid_len the size of the ICCID.
 * @param date the current date.
 * @param date_len the size of the date.
 * @param msg the byte array to verify.
 * @param msg_len the length of the message.
 *
 * @return OK if the authentication key was valid.
 *         ACP_KEY_ERR_BAD_KEY if the authentication key was invalid.
 *         ACP_KEY_ERR_INVALID_PARAM_LEN if any of the given lengths are invalid.
 *
 * @pre kt != NULL
 * @pre ks != NULL
 * @pre iccid != NULL
 * @pre date != NULL
 * @pre msg != NULL
 * @see acp_msg_cfg_activation
 */
E_EXPORT e_ret acp_key_verify(
        u8 *kt, u8 kt_len,
        u8 *ks, u8 ks_len,
        u8 *iccid, u8 iccid_len,
        u8 *date, u8 date_len,
        u8 *msg, u16 msg_len);

/**
 * Verifies if the given activation message contains a valid authentication key.
 *
 * The key is stored in the auth_key field of the activation message.
 *
 * @param kt the TCU secret key (Kt)
 * @param kt_len the length of the TCU key
 * @param iccid the ICCID of the TCU
 * @param iccid_len the size of the ICCID.
 * @param date the current date.
 * @param date_len the size of the date.
 * @param msg the message to verify.
 *
 * @return OK if the authentication key was valid.
 *         ACP_KEY_ERR_BAD_KEY if the authentication key was invalid.
 *
 * @pre kt != NULL
 * @pre iccid != NULL
 * @pre date != NULL
 * @pre msg != NULL
 * @see acp_msg_cfg_activation
 */
E_EXPORT e_ret acp_key_verify_msg(
        u8 *kt, u8 kt_len,
        u8 *iccid, u8 iccid_len,
        u8 *date, u8 date_len,
        acp_msg *msg);

/**
 * Computes the Ks to be included as the authentication key of the message.
 *
 * The key is stored in the space pointed by ks. The space must be allocated
 * before calling this function.
 *
 * @param kt the TCU secret key (Kt)
 * @param kt_len the length of the TCU key
 * @param iccid the ICCID of the TCU
 * @param iccid_len the size of the ICCID.
 * @param date the current date.
 * @param date_len the size of the date.
 * @param msg the byte content of the message to sign.
 * @param msg_len the size of the message.
 * @param ks a pointer where to store the computed Ks.
 * @param ks_len the size of the allocated memory space pointed by ks.
 *
 * @return OK if the key was successfully generated
 *
 * @pre kt != NULL
 * @pre iccid != NULL
 * @pre date != NULL
 * @pre msg != NULL
 * @post return != OK || authentication key stored on ks
 */
E_EXPORT e_ret acp_key_get(
        u8 *kt, u8 kt_len,
        u8 *iccid, u8 iccid_len,
        u8 *date, u8 date_len,
        u8 *msg, u16 msg_len,
        u8 *ks, u8 ks_len);

/**
 * Computes the Ks to be included as the authentication key of the message
 * and sets it.
 *
 * The key will be stored in the auth_key field of the msg.data.cfg_activation
 * structure. If the auth_key field does not point to NULL, the pointer will
 * first be freed.
 *
 * Required memory will be allocated for the key and should be later freed
 * by calling acp_msg_free or freeing the auth_key pointer.
 *
 * @param kt the TCU secret key (Kt)
 * @param kt_len the length of the TCU key
 * @param iccid the ICCID of the TCU
 * @param iccid_len the size of the ICCID.
 * @param date the current date.
 * @param date_len the size of the date.
 * @param acp_msg the activation message to sign.
 *
 * @return OK if the key was successfully generated and stored.
 *
 * @pre kt != NULL
 * @pre iccid != NULL
 * @pre date != NULL
 * @pre msg != NULL
 * @post return != OK || msg->data.cfg_activation.vehicle_desc.auth_key != NULL
 */
E_EXPORT e_ret acp_key_get_msg(
        u8 *kt, u8 kt_len,
        u8 *iccid, u8 iccid_len,
        u8 *date, u8 date_len,
        acp_msg *msg);

#ifdef __cplusplus
}
#endif

#endif /* __acp_key_h__ */

