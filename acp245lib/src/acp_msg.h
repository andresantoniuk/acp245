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
 * ACP 245 message description and processing functions.
 *
 * This file defines the structure of ACP messages as handled by this library
 * and provides functions to read and write messages from byte buffers.
 *
 * Information Elements are described in "acp_el.h".
 *
 * @file acp_msg.h
 * @date  03/13/2009 01:51:28 PM
 * @author Edantech
 * @see acp_el.h
 */
/*
 * Contributors:  Santiago Aguiar, santiago.aguiar@edantech.com
 */
#ifndef __acp_msg_h_
#define __acp_msg_h_

#ifdef E_ACP245_HAVE_E_LIBS
#include "e_port.h"
#include "e_buff.h"
#else
#include "acp_types.h"
#endif

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

#include "acp_el.h"
#include "acp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Application ID.
 * @see Section 1.1, Telematics Applications of [ACP245].
 */
typedef enum acp_msg_app_id {
    ACP_APP_ID_PROVISIONING             = 1,
    ACP_APP_ID_CONFIGURATION            = 2,
    ACP_APP_ID_REMOTE_VEHICLE_FUNCTION  = 6,
    ACP_APP_ID_VEHICLE_TRACKING         = 10,
    ACP_APP_ID_ALARM                    = 11
} acp_msg_app_id;

/**
 * ACP Message types.
 */
typedef enum acp_msg_type {
    /* Section 5, Provisioning Message Set */
    ACP_MSG_TYPE_PROV_UPD           = 1,
    ACP_MSG_TYPE_PROV_REPLY         = 3,
    /* The following are NOT used on 245 */
    ACP_MSG_TYPE_PROV_UPD_COMMIT    = 2,
    ACP_MSG_TYPE_PROV_REPLY_COMMIT  = 4,
    ACP_MSG_TYPE_PROV_REQUEST       = 5,
    ACP_MSG_TYPE_PROV_STATUS        = 6,

    /* Section 6, Fleet Management Configuration Message Set */
	ACP_MSG_TYPE_CFG_REPLY          = 3,
	ACP_MSG_TYPE_CFG_UPD_245        = 8,
	ACP_MSG_TYPE_CFG_REPLY_245      = 9,
	ACP_MSG_TYPE_CFG_ACT_245        = 10,
    /* The following are NOT used on 245 */
	ACP_MSG_TYPE_CFG_UPD            = 1,
	ACP_MSG_TYPE_CFG_UPD_COMMIT     = 2,
	ACP_MSG_TYPE_CFG_REPLY_COMMIT   = 4,
	ACP_MSG_TYPE_CFG_REQUEST        = 5,
	ACP_MSG_TYPE_CFG_STATUS         = 6,
	ACP_MSG_TYPE_CFG_EDIT           = 7,

    /* Section 7, Remote Vehicle Function Message Set */
	ACP_MSG_TYPE_FUNC_CMD           = 2,
	ACP_MSG_TYPE_FUNC_STATUS        = 3,
    /* The following are NOT used on 245 */
	ACP_MSG_TYPE_FUNC_REQ           = 1,

    /* Section 8, Vehicle Tracking Message Set */
	ACP_MSG_TYPE_TRACK_POS          = 2,
	ACP_MSG_TYPE_TRACK_REPLY        = 3,
    /* The following are NOT used on 245 */
	ACP_MSG_TYPE_TRACK_CMD          = 1,
	ACP_MSG_TYPE_TRACK_WITH_COMMIT  = 4,
	ACP_MSG_TYPE_TRACK_COMMIT       = 5,

    /* Section 9, Theft Alarm Message Set */
	ACP_MSG_TYPE_ALARM_NOTIF        = 1,
	ACP_MSG_TYPE_ALARM_REPLY        = 2,
	ACP_MSG_TYPE_ALARM_POS          = 3,
	ACP_MSG_TYPE_ALARM_KA           = 4,
	ACP_MSG_TYPE_ALARM_KA_REPLY     = 5

} acp_msg_type;

/**
 * @name Message Control Flag.
 * @see Section 4.1.7 of [ACP245]
 */
/*@{*/
/* Reserved                                 0x8 */
/** Dont Use TLV */
#define ACP_HDR_MSG_CTRL_DONT_USE_TLV       0x4
/** Set if the message length field is 2 bytes long */
#define ACP_HDR_MSG_CTRL_16BIT_LEN          0x2
/** Set if a response is expected. */
#define ACP_HDR_MSG_CTRL_RESP_EXP           0x1
/*@}*/

/**
 * Message Priority Flag.
 * @see Section 4.1.8 of [ACP245]
 */
typedef enum acp_msg_hdr_prio {
    ACP_HDR_MSG_PRIO_RESERVED   = 0,
    ACP_HDR_MSG_PRIO_ABORT      = 1,
    ACP_HDR_MSG_PRIO_PAUSE      = 2,
    ACP_HDR_MSG_PRIO_RESUME     = 3
} acp_msg_hdr_prio;

#define ACP_MSG_HDR_MAX_LEN                 6
/**
 * Message Header.
 * @see Section 4 of [ACP245]
 */
typedef struct acp_hdr {
    acp_msg_app_id app_id;
    bool test;
    acp_msg_type type;
    u8 version;
    /* msg_ctrl is only examined for ACP_HDR_MSG_CTRL_RESP_EXP flag on input */
    u8 msg_ctrl;
    acp_msg_hdr_prio msg_prio;
} acp_hdr;

/**
 * Provision Update Message #1 (From SO to TCU).
 * @see Section 5.2 of [ACP245]
 */
typedef struct acp_msg_prov_upd {
    acp_el_version version;
    u8 target_app_id;
    u8 appl_flg;
    u8 ctrl_flg1;
    u8 ctrl_flg2;
    /** @toggles ctrl_flg1,ACP_MSG_PROV_START_TIME_MASK */
    acp_el_timestamp start_time;
    /** @toggles ctrl_flg1,ACP_MSG_PROV_END_TIME_MASK */
    acp_el_timestamp end_time;
    /** @toggles ctrl_flg1,ACP_MSG_PROV_GRACE_TIME_MASK */
    acp_el_timestamp grace_time;
    acp_el_tcu_desc tcu_desc;
    /** @toggles ctrl_flg1,ACP_MSG_PROV_VEHICLE_DESC_MASK */
    acp_el_vehicle_desc vehicle_desc;
} acp_msg_prov_upd;

/**
 * Provision Reply Message #1 (From TCU to SO).
 * @see Section 5.3 of [ACP245]
 */
typedef struct acp_msg_prov_reply {
    acp_el_version version;
    u8 target_app_id;
    u8 appl_flg;
    u8 ctrl_flg1;
    u8 status;
    u8 tcu_resp;
    acp_el_error error;
    /** @toggles ctrl_flg1,ACP_MSG_PROV_VEHICLE_DESC_MASK */
    acp_el_vehicle_desc vehicle_desc;
} acp_msg_prov_reply;

/**
 * Configuration Update Message #2 ACP 245 (From SO to TCU).
 * @see Section 6.2 of [ACP245]
 */
typedef struct acp_msg_cfg_upd_245 {
    acp_el_version version;
    u8 target_app_id;
    u8 appl_flg;
    u8 ctrl_flg1;
    u8 ctrl_flg2;

    /** @toggles ctrl_flg1,ACP_MSG_PROV_START_TIME_MASK */
    acp_el_timestamp start_time;
    /** @toggles ctrl_flg1,ACP_MSG_PROV_END_TIME_MASK */
    acp_el_timestamp end_time;
    /** @toggles ctrl_flg1,ACP_MSG_PROV_GRACE_TIME_MASK */
    acp_el_timestamp grace_time;
    /** @toggles ctrl_flg1,ACP_MSG_PROV_VEHICLE_DESC_MASK */
    acp_el_vehicle_desc vehicle_desc;
    acp_el_tcu_desc tcu_desc;

    acp_el_tcu_data tcu_data;
} acp_msg_cfg_upd_245;

/**
 * Configuration Reply (From TCU to SO).
 * @see Section 6.3 of [ACP245]
 */
typedef struct acp_msg_cfg_reply {
    acp_el_version version;
    u8 target_app_id;
    u8 appl_flg;
    u8 ctrl_flg1;
    u8 status;
    u8 tcu_resp;

    acp_el_error error;
    /** @toggles ctrl_flg1,ACP_MSG_PROV_VEHICLE_DESC_MASK */
    acp_el_vehicle_desc vehicle_desc;
} acp_msg_cfg_reply;

/**
 * Configuration Reply #2 ACP 245 (From TCU to SO).
 * @see Section 6.4 of [ACP245]
 */
typedef struct acp_msg_cfg_reply_245 {
    acp_el_version version;
    u8 target_app_id;
    u8 appl_flg;
    u8 ctrl_flg1;
    u8 status;
    u8 tcu_resp;

    acp_el_tcu_data_error error;
    /** @toggles ctrl_flg1,ACP_MSG_PROV_VEHICLE_DESC_MASK */
    acp_el_vehicle_desc vehicle_desc;
} acp_msg_cfg_reply_245;

/**
 * Configuration TCU Service Activation/Deactivation Message ACP 245
 * (From SO to TCU).
 * @see Section 6.5 of [ACP245]
 */
typedef struct acp_msg_cfg_activation {
    acp_el_apn_cfg apn_cfg;
    acp_el_server_cfg server_cfg;
    u8 ctrl_byte;
    acp_el_vehicle_desc vehicle_desc;
} acp_msg_cfg_activation;

/**
 * Vehicle Function Command (From SO to TCU).
 * @see Section 7.2 of [ACP245]
 */
typedef struct acp_msg_func_cmd {
    acp_el_version version;
    acp_el_ctrl_func ctrl_func;
    acp_el_func_cmd func_cmd;
    acp_el_vehicle_desc vehicle_desc;
} acp_msg_func_cmd;

/**
 * Vehicle Function Status (From TCU to SO).
 * @see Section 7.3 of [ACP245]
 */
typedef struct acp_msg_func_status {
    acp_el_version version;
    acp_el_ctrl_func ctrl_func;
    /* function status ([ACP]17.37) is defined as the same IE as function command
    *([ACP]17.36,[ACP245]3.7) in ACP 245 */
    acp_el_func_cmd func_status;
    acp_el_error error;
    acp_el_vehicle_desc vehicle_desc;
} acp_msg_func_status;

/**
 * Vehicle Tracking Command (From SO to TCU).
 * @see Section 8.2 of [ACP245]
 */
typedef struct acp_msg_track_cmd {
    acp_el_version version;
    acp_el_ctrl_func ctrl_func;
    acp_el_func_cmd func_cmd;
    acp_el_vehicle_desc vehicle_desc;
} acp_msg_track_cmd;

/**
 * Vehicle Position Message (From TCU to SO).
 * @see Section 8.3 of [ACP245]
 */
typedef struct acp_msg_track_pos {
    acp_el_version version;
    acp_el_timestamp timestamp;
    acp_el_location location;
    acp_el_vehicle_desc vehicle_desc;
    acp_el_breakdown_status breakdown_status;
    acp_el_info_type info_type;
} acp_msg_track_pos;

/**
 * Vehicle Reply Message (From SO to TCU).
 * @see Section 8.4 of [ACP245]
 */
typedef struct acp_msg_track_reply {
    acp_el_version version;
    u8 confirmation;
    u8 transmit_unit;
    u8 ctrl_flg;
    acp_el_error error;
} acp_msg_track_reply;

/**
 * Theft Alarm Notification (From TCU to SO).
 * @see Section 9.2 of [ACP245]
 */
typedef struct acp_msg_alarm_notif {
    acp_el_version version;
    acp_el_timestamp timestamp;
    acp_el_location location;
    acp_el_vehicle_desc vehicle_desc;
    acp_el_breakdown_status breakdown_status;
    acp_el_info_type info_type;
} acp_msg_alarm_notif;

/**
 * Theft Alarm Reply (From SO to TCU).
 * @see Section 9.3 of [ACP245]
 */
typedef struct acp_msg_alarm_reply {
    acp_el_version version;
    u8 confirmation;
    u8 transmit_unit;
    u8 ctrl_flg;
    acp_el_error error;
} acp_msg_alarm_reply;

/**
 * Vehicle Position Message (TCU to SO).
 * @see Section 9.4 of [ACP245]
 */
typedef struct acp_msg_alarm_pos {
    acp_el_version version;
    acp_el_timestamp timestamp;
    acp_el_location location;
    acp_el_vehicle_desc vehicle_desc;
    acp_el_breakdown_status breakdown_status;
    acp_el_info_type info_type;
} acp_msg_alarm_pos;

/**
 * Message Keep Alive (TCU to SO).
 * @see Section 9.5 of [ACP245]
 */
typedef struct acp_msg_alarm_ka {
    acp_el_vehicle_desc vehicle_desc;
} acp_msg_alarm_ka;

/**
 * Message Keep Alive Reply (SO to TCU).
 * @see Section 9.6 of [ACP245]
 */
typedef struct acp_msg_alarm_ka_reply {
    acp_el_vehicle_desc vehicle_desc;
} acp_msg_alarm_ka_reply;

/** ACP245 Message Structure. */
typedef union acp_msg_data {
    acp_msg_prov_upd prov_upd;
    acp_msg_prov_reply prov_reply;
    acp_msg_cfg_activation cfg_activation;
    acp_msg_cfg_upd_245 cfg_upd_245;
    acp_msg_cfg_reply cfg_reply;
    acp_msg_cfg_reply_245 cfg_reply_245;
    acp_msg_func_cmd func_cmd;
    acp_msg_func_status func_status;
    acp_msg_track_cmd track_cmd;
    acp_msg_track_pos track_pos;
    acp_msg_track_reply track_reply;
    acp_msg_alarm_notif alarm_notif;
    acp_msg_alarm_reply alarm_reply;
    acp_msg_alarm_pos alarm_pos;
    acp_msg_alarm_ka alarm_ka;
    acp_msg_alarm_ka_reply alarm_ka_reply;
} acp_msg_data;

typedef struct acp_msg {
    /** Message header */
    acp_hdr hdr;

   /**
     * Message payload.
     *
     * Before accessing this field, check that the message application ID and
     * message type on the header match the type of the payload field that you
     * will access, otherwise it's value is undefined.
     */
    acp_msg_data data;
} acp_msg;

#ifdef E_ACP245_HAVE_E_LIBS
/* Functions only included if library is packed and compiled with e_libs */

/**
 * Reads an ACP message from the given buffer.
 *
 * If there's an error reading the message, an error code will be returned
 * and the buffer will be unchanged. Otherwise, the buffer position will be
 * updated, reflecting how many bytes were consumed.
 *
 * If this function returns OK, you must call acp_msg_free when you no longer
 * need the message.
 *
 * This function calls @ref acp_msg_init on your behalf, you don't need to
 * initialize the message before calling it.
 *
 * To use this function, you need to have e_libs library and header files and
 * define the E_ACP245_HAVE_E_LIBS flag when compiling the code.
 *
 * @return OK or an error code, as defined on acp_err.h
 * @param buff the buffer.
 * @param acp_msg a pointer to a message struct.
 * @pre buff != NULL
 * @pre msg != NULL
 */
E_EXPORT e_ret acp_msg_read(e_buff *buff, acp_msg *msg);

/**
 * Write an ACP message to the given buffer.
 *
 * If there's an error writing the message, an error code will be returned
 * and the buffer will be unchanged. Otherwise, the buffer limit will be
 * updated, reflecting how many bytes were written to the buffer.
 *
 * To use this function, you need to have e_libs library and header files and
 * define the E_ACP245_HAVE_E_LIBS flag when compiling the code.
 *
 * @return OK or an error code, as defined on acp_err.h
 * @param buff the buffer.
 * @param msg a pointer to a message struct.
 * @pre buff != NULL
 * @pre msg != NULL
 */
E_EXPORT e_ret acp_msg_write(e_buff *buff, acp_msg* msg);

#endif
/* Functions include on all version of the library */

/**
 * Initializes an ACP message struct so it can be safely used.
 *
 * If you are creating the message (instead of reading it with acp_msg_read),
 * you must call this function before calling on @ref acp_msg_write and
 * @ref acp_msg_write_data.
 *
 * You can pass an unknown application ID or message type. In that case, calls
 * to functions that read, write or operate on the message will fail if they
 * do not support that application ID or message type.
 *
 * @param msg the ACP message structure to initialize
 * @param app_id the application ID.
 * @param type the message type.
 * @return OK if the message was successfully initialized.
 * @pre msg != NULL
 * @post msg.hdr.app_id == app_id
 * @post msg.hdr.type == type
 */
E_EXPORT e_ret acp_msg_init(acp_msg *msg, acp_msg_app_id app_id, acp_msg_type type);

/**
 * Reads an ACP message from the byte array.
 *
 * If there's an error reading the message, an error code will be returned, and
 * the value of the readed parameter is undefined. Otherwise, the readed
 * parameter is not NULL, its value will be the number of bytes readed from the
 * the byte array.
 *
 * If this function returns OK, you must call acp_msg_free when you no longer
 * need the message.
 *
 * This function calls @ref acp_msg_init on your behalf, you don't need to
 * initialize the message before calling it.
 *
 * @return OK or an error code, as defined on acp_err.h
 * @param data the byte array.
 * @param data_len the length of the byte array.
 * @out readed if return is OK, the number of bytes readed, undefined
 * otherwise. If NULL, the parameter will be ignored.
 * @param msg a pointer to ACP message.
 * @pre data != NULL
 * @pre msg != NULL
 */
E_EXPORT e_ret acp_msg_read_data(u8* data, u32 data_len, u32 *readed, acp_msg *msg);

/**
 * Writes an ACP message to a byte array.
 *
 * If there's an error reading the message, an error code will be returned, and
 * the value of the written parameter is undefined. Otherwise, if the written
 * parameter is not NULL, its value will be the number of bytes written to
 * the the byte array.
 *
 * @return OK or an error code, as defined on acp_err.h
 * @param data the byte array.
 * @param data_len the length of the byte array.
 * @out written if return is OK, the number of bytes written, 
 * undefined otherwise. If NULL, the parameter will be ignored.
 * @param msg a pointer to a valid ACP message.
 * @pre data != NULL
 * @pre msg != NULL
 */
E_EXPORT e_ret acp_msg_write_data(u8* data, u32 data_len, u32 *written, acp_msg* msg);

/**
 * Returns if a message with the given reply_id and reply_type identify a
 * reply for a message with the given application id and message type.
 *
 * @return TRUE if it's a valid reply, FALSE otherwise.
 * @param id the application ID of the message.
 * @param type the message type of the message.
 * @param reply_id the application ID of the reply message.
 * @param reply_type the message type of the reply message.
 */
E_EXPORT bool acp_msg_is_reply_codes(acp_msg_app_id id, acp_msg_type type, acp_msg_app_id reply_id, acp_msg_type reply_type);

/**
 * Returns if a message of the given type for the given application ID can
 * be sent from a TCU.
 *
 * @return TRUE if the message can be sent from a TCU.
 * @param id the application ID of the message.
 * @param type the message type of the message.
 */
E_EXPORT bool acp_msg_is_tcu_message(acp_msg_app_id id, acp_msg_type type);

/**
 * Returns if a message of the given type for the given application ID can
 * be sent from a Service Operator.
 *
 * @return TRUE if the message can be sent from a SO.
 * @param id the application ID of the message.
 * @param type the message type of the message.
 */
E_EXPORT bool acp_msg_is_so_message(acp_msg_app_id id, acp_msg_type type);

/**
 * Frees the internal structures of the ACP message.
 *
 * When reading an ACP message with acp_msg_read or acp_msg_read_data, resources
 * may be allocated for some fields (ie. string information elements with variable
 * length). By calling this function, these resources will be deallocated.
 *
 * You should call this function only with an acp_msg structure that was
 * previously used on a cal to acp_msg_read or acp_msg_read_data.
 *
 * After calling this function, the acp_msg structure can be reused on a new
 * acp_msg_read or acp_msg_read_data.
 *
 * @param msg a pointer to an ACP message structure.
 * @post msg invalid
 */
E_EXPORT void acp_msg_free(acp_msg *msg);

#ifdef __cplusplus
}
#endif

#endif
