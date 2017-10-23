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
 * ACP 245 information element description and processing functions.
 *
 * This file defines the structure of ACP information elements as handled by
 * this library and provides functions to read and write information elements
 * from byte buffers.
 *
 * The functions exported by this file are not generally useful to external
 * applications. Users of the ACP 245 library should use the functions exported on
 * acp_msg.h instead of this one.
 *
 * @file acp_el.h
 * @date  03/13/2009 02:01:17 PM
 * @author Edantech
 */
/*
 * Contributors:  Santiago Aguiar <santiago.aguiar@edantech.com>
 */
#ifndef __acp_el_h_
#define __acp_el_h_

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

#ifdef E_ACP245_HAVE_E_LIBS
/* Include e_libs only if packed with the ACP library. */
#include "e_port.h"
#include "e_buff.h"
#else
#include "acp_types.h"
#endif

#include "acp_err.h"
#include "acp_ie.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Indicates the presence state of an element.
 *
 * This flag is included on all the elements that support being empty (length = 0)
 * or not included because the message was truncated or they were explicitely not
 * included by using a control flag.
 */
typedef enum acp_el_presence {
    /** The element was not included on the message (truncated or explicitely
     * not included by using a control flag) */
    ACP_EL_NOT_PRESENT  = 0,
    /** The element was included with length 0 */
    ACP_EL_EMPTY        = 1,
    /** The elment was included with length > 0 */
    ACP_EL_PRESENT      = 2
} acp_el_presence;

/**
 * Controlled Entity ID.
 * @see Section 3.6.1 of [ACP245]
 */
typedef enum acp_el_ctrl_entity {
	ACP_ENT_ID_DOOR_LOCKS			        = 0,
	ACP_ENT_ID_VEHICLE_TRACK			    = 1,
	ACP_ENT_ID_COVERT_MODE			        = 2,
	ACP_ENT_ID_MICROPHONE			        = 3,
    /* RESERVED                             4 */
	ACP_ENT_ID_TRANSMIT_INT			        = 5,
    /* RESERVED                             6 */
	ACP_ENT_ID_VEHICLE_TRACK_WITH_COMMIT    = 7,
	ACP_ENT_ID_VEHICLE_TRACK_COMMIT			= 8,
	ACP_ENT_ID_ALARMS			            = 9,
	ACP_ENT_ID_IMMOBILIZE			        = 10,
	ACP_ENT_ID_REMOTE_DOOR_LOCK			    = 11,
	ACP_ENT_ID_PRIMARY_ANTENNA			    = 12,
	ACP_ENT_ID_CALL_SO			            = 13,
	ACP_ENT_ID_CALL_SO_DATA			        = 14,
	ACP_ENT_ID_FUEL_PUMP_BLOCK			    = 15,
	ACP_ENT_ID_SIREN			            = 16,
	ACP_ENT_ID_POS_HISTORY		            = 17,
    /* RESERVED                             17..127 */
	ACP_ENT_ID_VEHICLE_BLOCK			    = 128
    /* RESERVED                             130..255 */
} acp_el_ctrl_entity;

/**
 * Transmit Unit.
 * @see Section 3.6.2 of [ACP245]
 */
typedef enum acp_el_transmit_unit {
	ACP_EL_TIME_UNIT_SECOND			    = 0,
	ACP_EL_TIME_UNIT_MINUTE			    = 1,
	ACP_EL_TIME_UNIT_HOUR			    = 2,
	ACP_EL_TIME_UNIT_ONEMORE			= 3,
	ACP_EL_TIME_UNIT_ONLYONE			= 4
} acp_el_transmit_unit;

/**
 * Version Element.
 * @see Section 3.1 of [ACP245]
 */
typedef struct acp_el_version {
    acp_el_presence present;
    u8 car_manufacturer;
    u8 tcu_manufacturer;
    u8 major_hard_rel;
    u8 major_soft_rel;
} acp_el_version;

/**
 * Timestamp Element.
 * @see Section 3.2 of [ACP245]
 */
typedef struct acp_el_timestamp {
    u16 year;
    u8 month;
    u8 day;
    u8 hour;
    u8 minute;
    u8 second;
} acp_el_timestamp;

/**
 * TCU Descriptor Element.
 * @see Section 3.3 of [ACP245]
 */
typedef struct acp_el_tcu_desc {
    acp_el_presence present;
    u8 device_id;
    bool is_str;
    acp_ie_any version;
} acp_el_tcu_desc;

/**
 * Vehicle Descriptor Element.
 * @see Section 3.4 of [ACP245]
 */
typedef struct acp_el_vehicle_desc {
    acp_el_presence present;

    u8 flg1;
    /** @toggles flg1,ACP_VEHICLE_DESC_FLG_ADDL_FLG */
    u8 flg2;
    /** @toggles flg1,ACP_VEHICLE_DESC_FLG1_LANG */
    u8 lang;
    /** @toggles flg2,ACP_VEHICLE_DESC_FLG2_MODEL_YEAR */
    u8 model_year;
    /** @toggles flg1,ACP_VEHICLE_DESC_FLG1_VIN */
    ascii* vin;         /* VIN max len 17 */
    /** @toggles flg1,ACP_VEHICLE_DESC_FLG1_TCU_SERIAL */
    acp_ie_any tcu_serial;
    /** @toggles flg1,ACP_VEHICLE_DESC_FLG1_LICENSE_PLATE */
    ascii* license_plate;
    /** @toggles flg1,ACP_VEHICLE_DESC_FLG1_VEHICLE_COLOR */
    ascii* vehicle_color;
    /** @toggles flg1,ACP_VEHICLE_DESC_FLG1_VEHICLE_MODEL */
    ascii* vehicle_model;
    /** @toggles flg1,ACP_VEHICLE_DESC_FLG1_IMEI */
    ascii* imei;        /* IMEI max len 15 */
    /** @toggles flg2,ACP_VEHICLE_DESC_FLG2_SIM_CARD_ID */
    ascii* iccid;       /* ICCID max len 20, added by [245] */
    u8 auth_key_len;
    /** @toggles flg2,ACP_VEHICLE_DESC_FLG2_AUTH_KEY */
    u8* auth_key;
} acp_el_vehicle_desc;

/**
 * Error Element.
 * @see Section 3.5 of [ACP245]
 */
typedef struct acp_el_error {
    u8 code;
} acp_el_error;

/**
 * Control Function Element.
 * @see Section 3.6 of [ACP245]
 */
typedef struct acp_el_ctrl_func {
    acp_el_ctrl_entity entity_id;

    /* if TRUE, include transmit_* in IE
     * @force_include
     * */
    bool transmit_present;
    /** @toggles transmit_present,TRUE */
    acp_el_transmit_unit transmit_unit;
    u8 transmit_interval;
} acp_el_ctrl_func;

/**
 * Raw Data Element.
 * @see Section 3.7.2 of [ACP245]
 */
typedef struct acp_el_raw_data {
    acp_el_presence present;
    u32 data_len;
    u8* data;
} acp_el_raw_data;

/**
 * Function Command Element.
 * @see Section 3.7 of [ACP245]
 */
typedef struct acp_el_func_cmd {
    u8 cmd;
    acp_el_raw_data raw_data;
} acp_el_func_cmd;

/** Maximum number of satellite IDs stored by the library. */
#define ACP_EL_GPS_RAW_DATA_SAT_MAX         (16)

/** Maximum number of location delta items stored by the library */
#define ACP_EL_LOC_DELTA_MAX                (10)

/**
 * GPS Raw Data Element.
 * @see Section 3.8.1 of [ACP245]
 */
typedef struct acp_el_gps_raw_data {
    acp_el_presence present;

    /* merged Area Location Coding (3.8.2) with GPSRawData to avoid unnecesary
     * indirection */
    u8 flg1;
    u8 flg2;
    u8 area_type;
    u8 location_type;
    u32 time_diff;
    /** longitude in milliarcsecond (1/3600000 degrees) */
    s32 lon;
    /** latitude in milliarcsecond (1/3600000 degrees) */
    s32 lat;
    /** altitude in meters */
    u16 alt;
    u8 pos_uncert;

    /** 3.8.11.1, 0 = use K, 1 = use DOP */
    bool hdop;

    u8 head_uncert;

    /** heading in multiples of 15 degrees */
    u8 heading;

    u8 dist_unit;
    u8 time_unit;

    /** velocity, unit given by dist_unit flag */
    u8 velocity;

    /** number of available satellites */
    u8 satellites_avail;

    /**
     * Number of reported satellite IDs.
     * May be different from the number of available satellites because
     * of truncation. This case is specifically covered in [ACP245].
     */
    u8 satellites_cnt;
    u8 satellites[ACP_EL_GPS_RAW_DATA_SAT_MAX];
} acp_el_gps_raw_data;

/**
 * Dead Reckoning Element.
 * @see Section 3.8.16 of [ACP245]
 */
typedef struct acp_el_dead_reck {
    acp_el_presence present;
    s32 lat;
    s32 lon;
} acp_el_dead_reck;

/**
 * Location Delta Coding Element.
 * @see Section 3.8.17 of [ACP245]
 */
typedef struct acp_el_loc_delta_latlon {
    u8 lon;
    u8 lat;
} acp_el_loc_delta_latlon;

typedef struct acp_el_loc_delta {
    acp_el_presence present;
    u8 delta_cnt;
    acp_el_loc_delta_latlon delta[ACP_EL_LOC_DELTA_MAX];
} acp_el_loc_delta;

/**
 * Location Element.
 * @see Section 3.8 of [ACP245]
 */
typedef struct acp_el_location {
    acp_el_gps_raw_data curr_gps;
    acp_el_gps_raw_data prev_gps;
    acp_el_dead_reck dead_reck;
    acp_el_loc_delta loc_delta;
} acp_el_location;

/**
 * @name Number of allowed breakdown status source fields
 */
/*@{*/
/** Maximum number of breakdown status elements. */
#define ACP_EL_BREAKDOWN_STATUS_MAX_SOURCE      (5)
/** Minimum number of breakdown status elements. */
#define ACP_EL_BREAKDOWN_STATUS_MIN_SOURCE      (1)
/*@}*/
/**
 * Breakdown Status Element.
 * @see Section 3.9 of [ACP245]
 */
typedef struct acp_el_breakdown_status {
    acp_el_presence present;
    u8 source_cnt;
    u8 source[ACP_EL_BREAKDOWN_STATUS_MAX_SOURCE];
    u8 sensor;
    u32 data_len;
    u8* data;
} acp_el_breakdown_status;

/**
 * Information Type Element.
 * @see Section 3.10 of [ACP245]
 */
typedef struct acp_el_info_type {
    acp_el_presence present;
    u8 type;
    u32 data_len;
    u8* data;
} acp_el_info_type;

/**
 * TCU Data Element Item.
 * @see Section 3.11 of [ACP245]
 */
typedef struct acp_el_tcu_data_item {
    u16 type;
    /* 245 defines a max data size of 255 */
    u8 data_len;
    u8* data;
} acp_el_tcu_data_item;

/**
 * TCU Data Element.
 * @see Section 3.11 of [ACP245]
 */
typedef struct acp_el_tcu_data {
    u8 items_cnt;
    acp_el_tcu_data_item *items;
} acp_el_tcu_data;

/**
 * TCU Data Error Element Item.
 * @see Section 3.12 of [ACP245]
 */
typedef struct acp_el_tcu_data_error_item {
    u16 type;
    /* 245 defines a max data size of 255 */
    u8 data_len;
    u8* data;
    acp_el_error error;
} acp_el_tcu_data_error_item;

/**
 * TCU Data Error Element.
 * @see Section 3.12 of [ACP245]
 */
typedef struct acp_el_tcu_data_error {
    u8 items_cnt;
    acp_el_tcu_data_error_item *items;
} acp_el_tcu_data_error;

/**
 * APN Configuration Element
 * @see Section 6.5.1.2 of [ACP245]
 */
typedef struct acp_el_apn_cfg {
    acp_el_presence present;
    ascii *address;
    ascii *login;
    ascii *password;
} acp_el_apn_cfg;

/**
 * Server Configuration Element.
 * @see Section 6.5.1.3 of [ACP245]
 */
typedef struct acp_el_server_cfg {
    acp_el_presence present;
    /** @type IP */
    u32 server_1;
    u16 port_1;
    /** @type IP */
    u32 server_2;
    u16 port_2;
    u8 proto_id;
} acp_el_server_cfg;

#ifdef E_ACP245_HAVE_E_LIBS
/* Functions only included if library is packed and compiled with e_libs */

/**
 * @name Information Element processing functions.
 *
 * These functions read and write information elements from an e_buff, and
 * manage information element resources.
 *
 * These function should not be called directly by third party code, call
 * acp_msg_read and acp_msg_read_data instead.
 *
 * To use this functions, you need to have e_libs library and header files and
 * define the E_ACP245_HAVE_E_LIBS flag when compiling the code.
 *
 * @see acp_msg_read
 * @see acp_msg_read_data
 * @internal
 */
/*@{*/
E_EXPORT e_ret acp_el_skip_while_flag_sz(e_buff *buff, u8 flg_msk, u32 sz);
E_EXPORT e_ret acp_el_skip_while_flag(e_buff *buff, u8 flg_msk);

E_EXPORT void acp_el_free_version(acp_el_version *el);
E_EXPORT e_ret acp_el_read_version(e_buff *buff, acp_el_version *el);
E_EXPORT e_ret acp_el_write_version(e_buff *buff, acp_el_version *el);

E_EXPORT void acp_el_free_timestamp(acp_el_timestamp *el);
E_EXPORT e_ret acp_el_read_timestamp(e_buff *buff, acp_el_timestamp *el);
E_EXPORT e_ret acp_el_write_timestamp(e_buff *buff, acp_el_timestamp *el);

E_EXPORT void acp_el_free_tcu_desc(acp_el_tcu_desc *el);
E_EXPORT e_ret acp_el_read_tcu_desc(e_buff *buff, acp_el_tcu_desc *el);
E_EXPORT e_ret acp_el_write_tcu_desc(e_buff *buff, acp_el_tcu_desc *el);

E_EXPORT void acp_el_free_vehicle_desc(acp_el_vehicle_desc *el);
E_EXPORT e_ret acp_el_read_vehicle_desc(e_buff *buff, acp_el_vehicle_desc *el);
E_EXPORT e_ret acp_el_write_vehicle_desc(e_buff *buff, acp_el_vehicle_desc *el);

E_EXPORT void acp_el_free_error(acp_el_error *el);
E_EXPORT e_ret acp_el_read_error(e_buff *buff, acp_el_error *el);
E_EXPORT e_ret acp_el_write_error(e_buff *buff, acp_el_error *el);
E_EXPORT u32 acp_el_size_error(acp_el_error *el);

E_EXPORT void acp_el_free_ctrl_func(acp_el_ctrl_func *el);
E_EXPORT e_ret acp_el_read_ctrl_func(e_buff *buff, acp_el_ctrl_func *el);
E_EXPORT e_ret acp_el_write_ctrl_func(e_buff *buff, acp_el_ctrl_func *el);

E_EXPORT void acp_el_free_raw_data(acp_el_raw_data *el);
E_EXPORT e_ret acp_el_read_raw_data(e_buff *buff, acp_el_raw_data *el);
E_EXPORT e_ret acp_el_write_raw_data(e_buff *buff, acp_el_raw_data *el);

E_EXPORT void acp_el_free_func_cmd(acp_el_func_cmd *el);
E_EXPORT e_ret acp_el_read_func_cmd(e_buff *buff, acp_el_func_cmd *el);
E_EXPORT e_ret acp_el_write_func_cmd(e_buff *buff, acp_el_func_cmd *el);

E_EXPORT void acp_el_free_loc_delta(acp_el_loc_delta *el);
E_EXPORT e_ret acp_el_read_loc_delta(e_buff *buff, acp_el_loc_delta *el);
E_EXPORT e_ret acp_el_write_loc_delta(e_buff *buff, acp_el_loc_delta *el);

E_EXPORT void acp_el_free_dead_reck(acp_el_dead_reck *el);
E_EXPORT e_ret acp_el_read_dead_reck(e_buff *buff, acp_el_dead_reck *el);
E_EXPORT e_ret acp_el_write_dead_reck(e_buff *buff, acp_el_dead_reck *el);

E_EXPORT void acp_el_free_location_coding(acp_el_gps_raw_data *el);
E_EXPORT e_ret acp_el_read_location_coding(e_buff *buff, acp_el_gps_raw_data *el);
E_EXPORT e_ret acp_el_write_location_coding(e_buff *buff, acp_el_gps_raw_data *el);

E_EXPORT void acp_el_free_gps_raw_data(acp_el_gps_raw_data *el);
E_EXPORT e_ret acp_el_read_gps_raw_data(e_buff *buff, acp_el_gps_raw_data *el);
E_EXPORT e_ret acp_el_write_gps_raw_data(e_buff *buff, acp_el_gps_raw_data *el);

E_EXPORT void acp_el_free_location(acp_el_location *el);
E_EXPORT e_ret acp_el_read_location(e_buff *buff, acp_el_location *el);
E_EXPORT e_ret acp_el_write_location(e_buff *buff, acp_el_location *el);

E_EXPORT void acp_el_free_breakdown_status(acp_el_breakdown_status *el);
E_EXPORT e_ret acp_el_read_breakdown_status(e_buff *buff, acp_el_breakdown_status *el);
E_EXPORT e_ret acp_el_write_breakdown_status(e_buff *buff, acp_el_breakdown_status *el);

E_EXPORT void acp_el_free_info_type(acp_el_info_type *el);
E_EXPORT e_ret acp_el_read_info_type(e_buff *buff, acp_el_info_type *el);
E_EXPORT e_ret acp_el_write_info_type(e_buff *buff, acp_el_info_type *el);

E_EXPORT void acp_el_free_tcu_data(acp_el_tcu_data *el);
E_EXPORT e_ret acp_el_read_tcu_data(e_buff *buff, acp_el_tcu_data *el);
E_EXPORT e_ret acp_el_write_tcu_data(e_buff *buff, acp_el_tcu_data *el);

E_EXPORT void acp_el_free_tcu_data_error(acp_el_tcu_data_error *el);
E_EXPORT e_ret acp_el_read_tcu_data_error(e_buff *buff, acp_el_tcu_data_error *el);
E_EXPORT e_ret acp_el_write_tcu_data_error(e_buff *buff, acp_el_tcu_data_error *el);

E_EXPORT void acp_el_free_apn_cfg(acp_el_apn_cfg *el);
E_EXPORT e_ret acp_el_read_apn_cfg(e_buff *buff, acp_el_apn_cfg *el);
E_EXPORT e_ret acp_el_write_apn_cfg(e_buff *buff, acp_el_apn_cfg *el);

E_EXPORT void acp_el_free_server_cfg(acp_el_server_cfg *el);
E_EXPORT e_ret acp_el_read_server_cfg(e_buff *buff, acp_el_server_cfg *el);
E_EXPORT e_ret acp_el_write_server_cfg(e_buff *buff, acp_el_server_cfg *el);
/*@}*/

#endif

/*
 * Information Element constants.
 */

/**
 * More flag.
 * @see Section 2.3 of [ACP245]
 */
#define ACP_MORE_FLG                         0x80

/**
 * @name Vehicle Descriptor Flags
 * @see Section 3.4.1 of [ACP245]
 */
/*@{*/
#define ACP_VEHICLE_DESC_FLG_ADDL_FLG        0x80
#define ACP_VEHICLE_DESC_FLG1_LANG           0x40
#define ACP_VEHICLE_DESC_FLG1_VIN            0x20
#define ACP_VEHICLE_DESC_FLG1_TCU_SERIAL     0x10
#define ACP_VEHICLE_DESC_FLG1_VEHICLE_COLOR  0x08
#define ACP_VEHICLE_DESC_FLG1_VEHICLE_MODEL  0x04
#define ACP_VEHICLE_DESC_FLG1_LICENSE_PLATE  0x02
#define ACP_VEHICLE_DESC_FLG1_IMEI           0x01
#define ACP_VEHICLE_DESC_FLG2_MODEL_YEAR     0x40
#define ACP_VEHICLE_DESC_FLG2_SIM_CARD_ID    0x20
#define ACP_VEHICLE_DESC_FLG2_AUTH_KEY       0x10
/* Reserved                                 0x08..0x01 */
/*@}*/

/**
 * @name Area Location Status Flag 1
 * @see Section 3.8.3 of [ACP245]
 */
/*@{*/
/* Reserved                             0x40 */
#define ACP_LOCATION_FLG1_NO_3D_FIX     0x20
#define ACP_LOCATION_FLG1_NO_2D_FIX     0x10
#define ACP_LOCATION_FLG1_INVALID_POS   0x08
#define ACP_LOCATION_FLG1_DIFF_GPS      0x04
#define ACP_LOCATION_FLG1_INVALID_HEAD  0x02
#define ACP_LOCATION_FLG1_ALMANAC_BAD   0x01
/*@}*/

/**
 * @name Area Location Status Flag 2
 * @see Section 3.8.4 of [ACP245]
 */
/*@{*/
/* Reserved                             0x40 */
#define ACP_LOCATION_FLG2_NEW_GPS_DATA  0x20
/* Reserved                             0x10 */
/* Reserved                             0x08 */
#define ACP_LOCATION_FLG2_HEAD_MASK     0x07
#define ACP_LOCATION_NORTH              0
#define ACP_LOCATION_NORTH_EAST         1
#define ACP_LOCATION_EAST               2
#define ACP_LOCATION_SOUTH_EAST         3
#define ACP_LOCATION_SOUTH              4
#define ACP_LOCATION_SOUTH_WEST         5
#define ACP_LOCATION_WEST               6
#define ACP_LOCATION_NORTH_WEST         7
/*@}*/

/**
 * @name Area Type
 * @see Section 3.8.5 of [ACP245]
 */
/*@{*/
#define ACP_LOCATION_POINT_1_MILLIARC   0
#define ACP_LOCATION_POINT_100_MILLIARC 1
/*@}*/

/**
 * Location Type Coding
 * @see Section 3.8.6 of [ACP245]
 */
#define ACP_LOCATION_WGS_84             0
/* Reserved                             1..7 */

/**
 * @name Distance Flag
 * @see Section 3.8.14 of [ACP245]
 */
/*@{*/
#define ACP_LOCATION_DIST_UNIT_ND       0
#define ACP_LOCATION_DIST_UNIT_KM       1
#define ACP_LOCATION_DIST_UNIT_MI       2
/*@}*/

/**
 * @name Time Flag
 * @see Section 3.8.15 of [ACP245]
 */
/*@{*/
#define ACP_LOCATION_TIME_UNIT_SECONDS  0
#define ACP_LOCATION_TIME_UNIT_MINUTES  1
#define ACP_LOCATION_TIME_UNIT_HOURS    2
/* Reserved                             3 */
/*@}*/

/**
 * @name Device ID
 * @see Section 3.3.1 of [ACP245]
 */
/*@{*/
#define ACP_EL_TCU_DEVICE_ID_TCU_HARD_VER    1
#define ACP_EL_TCU_DEVICE_ID_TCU_MANUFACT    2
#define ACP_EL_TCU_DEVICE_ID_TCU_SOFT_VER    3
#define ACP_EL_TCU_DEVICE_ID_TCU_CAN_VER     4
#define ACP_EL_TCU_DEVICE_ID_ACP_TRANP_VER   5
#define ACP_EL_TCU_DEVICE_ID_ACP_APP_VER     6
/* Reserved                                  7..255 */
/*@}*/

/**
 * @name Valid Error Codes
 * @see Section 3.5.1 of [ACP245]
 */
/*@{*/
#define ACP_ERR_OK                      0
#define ACP_ERR_SERVICE_UNAVAILABLE     1
#define ACP_ERR_INCORRECT_APP           2
#define ACP_ERR_UNKNOWN_VERSION         3
#define ACP_ERR_UNKNOWN_MSG_TYPE        4
#define ACP_ERR_UNKNOWN_DATA_IN_MSG     5
#define ACP_ERR_UNKNOWN_TRANSPORT_VER   6
#define ACP_ERR_DATA_ERROR              7
#define ACP_ERR_SEC_VIOLATION           8
#define ACP_ERR_NO_ACC_NO_CUSTOMER      9
#define ACP_ERR_NO_ACC_NO_SERVICE       10
#define ACP_ERR_NO_ACC_AUTH_FAIL        11
#define ACP_ERR_NO_ACC_OTHER            12
#define ACP_ERR_INVALID_SES_ID          13
/* Reserved                             14 */
#define ACP_ERR_UNSUPPORTED_LANG        15
#define ACP_ERR_PROV_UPDATE_MISMATCH    16
#define ACP_ERR_PROV_SIM_ID_MISMATCH    17
#define ACP_ERR_PROV_UNABLE_TO_PROC     18
#define ACP_ERR_GENERAL                 19
#define ACP_ERR_NO_ACC_SIM              20
#define ACP_ERR_EEPROM                  21
#define ACP_ERR_INVALID_PHONE           22
#define ACP_ERR_VIN_MISMATCH            23
#define ACP_ERR_VEHICLE_MISMATCH        24
#define ACP_ERR_PROV_TOO_MANY_TARGETS   25
#define ACP_ERR_MISSING_PHONE           26
#define ACP_ERR_INVALID_ACT             27
#define ACP_ERR_INVALID_DEACT           28
#define ACP_ERR_BUFF_OVERFLOW           29
/* Available                            30..255 */
/*@}*/

/**
 * @name Function Command
 * @see Section 3.7.1 of [ACP245]
 */
/*@{*/
#define ACP_FUNC_CMD_PERMIT          0
#define ACP_FUNC_CMD_REJECT          1
#define ACP_FUNC_CMD_ENABLE          2
#define ACP_FUNC_CMD_DISABLE         3
#define ACP_FUNC_CMD_REQUEST         4
/*@}*/

/**
 * @name Function Status
 * @see Section 3.7.1 of [ACP245]
 */
/*@{*/
#define ACP_FUNC_STATE_PERMITTED      0
#define ACP_FUNC_STATE_REJECTED       1
#define ACP_FUNC_STATE_ENABLED        2
#define ACP_FUNC_STATE_DISABLED       3
#define ACP_FUNC_STATE_COMPLETED      4
/*@}*/

/**
 * @name Breakdown Source 1
 * @see Section 3.9.1 of [ACP245]
 */
/*@{*/
#define ACP_BKD_MANUALLY_ACTIVATED      0x40
#define ACP_BKD_VEHICLE_ROLLED          0x20
#define ACP_BKD_AIR_BAG_ACTIVATED       0x10
#define ACP_BKD_CRASH_SENSOR_ACTIVATED  0x08
#define ACP_BKD_FLOATING_CAR_DATA_INPUT 0x04
#define ACP_BKD_TOW_TRUCK_NEEDED        0x02
#define ACP_BKD_THEFT_ALARM             0x01
/*@}*/

/**
 * @name Breakdown Source 2
 * @see Section 3.9.1 of [ACP245]
 */
/*@{*/
#define ACP_BKD_VEHICLE_ON              0x40
#define ACP_BKD_VEHICLE_OFF             0x20
#define ACP_BKD_VEHICLE_MOVED           0x10
#define ACP_BKD_OTHER_SENSOR_ACT        0x08
#define ACP_BKD_RE_SEND_POS_TCU         0x04
#define ACP_BKD_RE_SEND_POS_SO          0x02
#define ACP_BKD_UNAUTH_VEHICLE_MOVE     0x01
/*@}*/

/**
 * @name Breakdown Source 3
 * @see Section 3.9.1 of [ACP245]
 */
/*@{*/
#define ACP_BKD_SIREN_ON                0x40
#define ACP_BKD_SIREN_OFF               0x20
#define ACP_BKD_MAIN_BATT_RECONN        0x10
#define ACP_BKD_MAIN_BARR_DISCONN       0x08
#define ACP_BKD_PANIC_ON                0x04
#define ACP_BKD_BLOCKING_ON             0x02
#define ACP_BKD_BLOCKING_OFF            0x01
/*@}*/

/**
 * @name Breakdown Sensor
 * @see Section 3.9.2 of [ACP245]
 */
/*@{*/
#define ACP_BKD_SENSOR_ADDL_FLG         0x80
#define ACP_BKD_SENSOR_ROLLOVER         0x40
#define ACP_BKD_SENSOR_FRONT            0x20
#define ACP_BKD_SENSOR_REAR             0x10
#define ACP_BKD_SENSOR_SIDE             0x08
#define ACP_BKD_SENSOR_ALARM            0x04
/* Reserved                             0x02 */
#define ACP_BKD_STATUS                  0x01
/*@}*/

/**
 * @name Information Type
 * @see Section 3.10.1 of [ACP245]
 */
/*@{*/
/* Reserved                             0 */
#define ACP_IT_VERBAL_INFO              1
#define ACP_IT_STOCK_INFO               2
#define ACP_IT_TRAVEL_ROUTE_INFO        3
#define ACP_IT_HOTEL_INFO               4
#define ACP_IT_TRAFFIC_INFO_VERBAL      5
#define ACP_IT_TRAFFIC_INFO_AUTOMATED   6
#define ACP_IT_ASCII_STRING             7
#define ACP_IT_POI                      8
#define ACP_IT_CARGO                    9
#define ACP_IT_PRIVATE                  10
#define ACP_IT_ENVIRONMENTAL            11
#define ACP_IT_TIMESTAMP                12
#define ACP_IT_COUNTRY_CODE             13
#define ACP_IT_MENU_BUTTON              14
/* Reserved                             15..127 */
/*@}*/

/* Section 4.1.6, Version */
/**
 * @name Version Field
 * @see Section 4.1.6 of [ACP245]
 */
/*@{*/
#define ACP_VER_1_2                     0
#define ACP_VER_1_2_1                   1
#define ACP_VER_1_2_2                   2
/* Available                            3..6 */
/* Reserved                             7 */
/*@}*/

/* Section 5.2.1.3, Message Fields */

/**
 * @name ApplFlg1 (appl_flg)
 * @see Section 5.2.1.3.2 of [ACP245]
 */
/*@{*/
#define ACP_MSG_PROV_NO_CHANGE          0
#define ACP_MSG_PROV_ACTIVATE           1
#define ACP_MSG_PROV_DEACTIVATE         2
#define ACP_MSG_PROV_CHANGE             3
/*@}*/

/**
 * @name ControlFlag1 (ctrl_flg1)
 * @see Section 5.2.1.3.3 of [ACP245]
 */
/*@{*/
#define ACP_MSG_PROV_ADDL_FLG_MASK      0x20
#define ACP_MSG_PROV_GRACE_TIME_MASK    0x10
#define ACP_MSG_PROV_START_TIME_MASK    0x08
#define ACP_MSG_PROV_END_TIME_MASK      0x04
#define ACP_MSG_PROV_VEHICLE_DESC_MASK  0x02
#define ACP_MSG_PROV_USE_COMMIT_MASK    0x01
/*@}*/

/**
 * @name ControlFlag2 (ctrl_flg1)
 * @see Section 5.2.1.3.4 of [ACP245]
 */
/*@{*/
#define ACP_MSG_PROV_USE_PROFILE(b)     (0==((b&0x70)>>4))
#define ACP_MSG_PROV_USE_SAMPLE(b)      (1==((b&0x70)>>4))
#define ACP_MSG_PROV_NUM_SAMPLES_MASK   0x08
#define ACP_MSG_PROV_NO_SAMPLE_UNIT(b)  (0==((b&0x07)))
#define ACP_MSG_PROV_SAMPLES_IN_MIN(b)  (1==((b&0x07)))
#define ACP_MSG_PROV_SAMPLES_IN_KM(b)   (2==((b&0x07)))
/*@}*/

/* Section 5.3.1.3.1, StatusFlag1 */
/**
 * @name StatusFlag1 (status)
 * @see Section 5.3.1.3.1 of [ACP245]
 */
/*@{*/
#define ACP_MSG_PROV_STATUS_ALREADY_PROV    0
#define ACP_MSG_PROV_STATUS_NOT_PROV        1
#define ACP_MSG_PROV_STATUS_SEE_ERROR       2
/* Reserved                                 3 */
/*@}*/

/**
 * ACP 245 Protocol ID
 * @see Section 6.5.1.3.1 of [ACP245]
 */
#define ACP_MSG_CFG_PROTO_ID_ACP245         (0)

/**
 * @name Control Value.
 * @see Section 6.5.1.4.1 of [ACP245]
 */
/*@{*/
#define ACP_MSG_CFG_CTRL_VALUE_ACTIVATE     0x40
/*@}*/

/**
 * @name TCU Response Flag.
 * @see Section 5.3.1.3.2 of [ACP245]
 */
/*@{*/
/* Reserved                                 0 */
#define ACP_MSG_PROV_TCU_INIT_MODE          1
#define ACP_MSG_PROV_TCU_RESP_TO_UPD        2
#define ACP_MSG_PROV_TCU_RESP_TO_COMMIT     3
/*@}*/

/* Section 8.4.1.3, Message Field Elements for Vehicle Position Message */

/**
 * @name Confirmation.
 * @see Section 8.4.1.3.1 of [ACP245]
 */
/*@{*/
#define ACP_MSG_TRACK_CONFIRM_ADDL_FLG      0x08
#define ACP_MSG_TRACK_CONFIRM_ACCEPTED      0x04
#define ACP_MSG_TRACK_CONFIRM_TURN_SPEAKER  0x02
#define ACP_MSG_TRACK_CONFIRM_PROCESS_START 0x01
/*@}*/

/**
 * @name EcallControlFlag2
 * @see Section 8.4.1.3.3 of [ACP245]
 */
/*@{*/
#define ACP_MSG_TRACK_CTRL_ADDL_FLG         0x80
#define ACP_MSG_TRACK_CTRL_CANCEL_ALARM     0x40
#define ACP_MSG_TRACK_CTRL_RE_SEND_REQ      0x20
#define ACP_MSG_TRACK_CTRL_DISABLE_VOICE    0x10
/* Reserved                                 0x08..0x01*/
/*@}*/

/**
 * @name Configuration Parameter Indexes. Tracking Service.
 * @see Section 12.1.1 of [ACP245]
 */
/*@{*/
#define ACP_PAR_TRACKING_TIMER              0x0011
#define ACP_PAR_TRACKING_TIMER_SLEEP        0x0012
#define ACP_PAR_GPS_QUALITY                 0x0013
#define ACP_PAR_DIR_CHANGE_THRES            0x0014
#define ACP_PAR_DIR_CHANGE_SPEED_THRES      0x0015
#define ACP_PAR_MAX_SPEED                   0x0016
#define ACP_PAR_ODOMETER_LIMIT              0x0017
#define ACP_PAR_ODOMETER_SAVE_THRES         0x0018
#define ACP_PAR_TRACKING_EVENT_TIMER        0x0019
/*@}*/

/**
 * @name Configuration Parameter Indexes. Immobilizer Service.
 * @see Section 12.1.2 of [ACP245]
 */
/*@{*/
#define ACP_PAR_IMMO_TIMER                  0x0021
/*@}*/

/**
 * @name Configuration Parameter Indexes. Anti-Theft Service.
 * @see Section 12.1.3 of [ACP245]
 */
/*@{*/
#define ACP_PAR_ANTI_THEFT_TIMER            0x0031
#define ACP_PAR_ANTI_THEFT_ENABLED          0x0032
/*@}*/

/**
 * @name Configuration Parameter Indexes. System Service.
 * @see Section 12.1.4 of [ACP245]
 */
/*@{*/
#define ACP_PAR_SLEEP_TIMER                 0x0041
#define ACP_PAR_PANIC_BUTTON_ENABLED        0x0042
#define ACP_PAR_TRANS_FLAG_ENABLED          0x0043
#define ACP_PAR_CONTRACT_FLAG_ENABLED       0x0044
#define ACP_PAR_AUTH_KEY                    0x0045
#define ACP_PAR_ACTIVATION_ENABLED          0x0046
#define ACP_PAR_LOCATION_ENABLED            0x0047
/*@}*/

/**
 * @name Configuration Parameter Indexes. Network Service.
 * @see Section 12.1.5 of [ACP245]
 */
/*@{*/
#define ACP_PAR_KEEPALIVE_TIMER             0x0051
#define ACP_PAR_TRANSPORT_PROTOCOL          0x0052
/*@}*/

/**
 * @name Configuration Parameter Indexes. Connectivity Service.
 * @see Section 12.1.6 of [ACP245]
 */
/*@{*/
#define ACP_PAR_APN                         0x0061
#define ACP_PAR_LOGIN                       0x0062
#define ACP_PAR_PASSWORD                    0x0063
#define ACP_PAR_SERVER_IP_1                 0x0064
#define ACP_PAR_SERVER_PORT_1               0x0065
#define ACP_PAR_SERVER_TRANSPORT_1          0x0066
#define ACP_PAR_SERVER_IP_2                 0x0067
#define ACP_PAR_SERVER_PORT_2               0x0068
#define ACP_PAR_SERVER_TRANSPORT_2          0x0069
#define ACP_PAR_SERVER_IP_3                 0x006A
#define ACP_PAR_SERVER_PORT_3               0x006B
#define ACP_PAR_SERVER_TRANSPORT_3          0x006C
#define ACP_PAR_SERVER_IP_4                 0x006D
#define ACP_PAR_SERVER_PORT_4               0x006E
#define ACP_PAR_SERVER_TRANSPORT_4          0x006F
#define ACP_PAR_SERVER_IP_5                 0x0070
#define ACP_PAR_SERVER_PORT_5               0x0071
#define ACP_PAR_SERVER_TRANSPORT_5          0x0072
#define ACP_PAR_SERVER_IP_6                 0x0073
#define ACP_PAR_SERVER_PORT_6               0x0074
#define ACP_PAR_SERVER_TRANSPORT_6          0x0075
/*@}*/

/**
 * @name Configuration Parameter Indexes. Power Service.
 * @see Section 12.1.7 of [ACP245]
 */
/*@{*/
#define ACP_PAR_BATT_MIN_VOLT               0x0081
#define ACP_PAR_BATT_MAX_VOLT               0x0082
#define ACP_PAR_BATT_HYST_VOLT              0x0083
#define ACP_PAR_BATT_BKUP_MIN_VOLT          0x0084
#define ACP_PAR_BATT_BKUP_RECHARGE          0x0085
#define ACP_PAR_BATT_BKUP_CHARGED           0x0086
#define ACP_PAR_BATT_BKUP_HYST_VOLT         0x0087
#define ACP_PAR_BATT_BKUP_MIN_TEMP          0x0088
#define ACP_PAR_BATT_BKUP_MAX_TEMP          0x0089
#define ACP_PAR_BATT_BKUP_HYST_TEMP         0x008A
#define ACP_PAR_BATT_BKUP_TIMER_1           0x008B
#define ACP_PAR_BATT_BKUP_TIMER_2           0x008C
#define ACP_PAR_BATT_BKUP_MAX_FAIL_COUNT    0x008D
#define ACP_PAR_BATT_BKUP_VALIDITY_YEAR     0x008E
#define ACP_PAR_BATT_BKUP_VALIDITY_MONTH    0x008F
/*@}*/

/**
 * @name Configuration Parameter Indexes. Alarm Service.
 * @see Section 12.1.8 of [ACP245]
 */
/*@{*/
#define ACP_PAR_IGNITION_ACT_TIME           0x0092
#define ACP_PAR_PANIC_ACT_TIME              0x0095
#define ACP_PAR_GPS_ERROR_ACT_TIME          0x0098
#define ACP_PAR_BATT_LOW_ACT_TIME           0x009B
#define ACP_PAR_BATT_BKUP_TEMP_ACT_TIME     0x009E
#define ACP_PAR_ANTI_THEFT_ACT_TIME         0x00A2
#define ACP_PAR_SLEEP_ACT_TIME              0x00A5
#define ACP_PAR_POS_ACT_TIME                0x00A8
#define ACP_PAR_GSM_WEAK_ACT_TIME           0x00AB
#define ACP_PAR_BATT_BKUP_LOW_ACT_TIME      0x00AE
#define ACP_PAR_BATT_BKUP_FAIL_ACT_TIME     0x00B2
#define ACP_PAR_BATT_BKUP_EOL_ACT_TIME      0x00B5
#define ACP_PAR_OUTPUT_SHORT_ACT_TIME       0x00B8
#define ACP_PAR_MAX_SPEED_ACT_TIME          0x00BB
#define ACP_PAR_COURSE_CHANGE_ACT_TIME      0x00BE
#define ACP_PAR_GPS_SAT_DROP_ACT_TIME       0x00C2
#define ACP_PAR_GSM_ANT_CUT_ACT_TIME        0x00C5
#define ACP_PAR_ALARM_ENABLED               0x00C6
/*@}*/

/**
 * @name Configuration Parameter Indexes. FOTA Service.
 * @see Section 12.1.9 of [ACP245]
 */
/*@{*/
#define ACP_PAR_FOTA_ENABLED                0x00D1
/*@}*/

/**
 * @name Configuration Parameter Indexes. Custom parameters.
 * @see Section 12.1.10 of [ACP245]
 */
/*@{*/
/* 0x0100 ... 0x01FF reserved */
/*@}*/

#ifdef __cplusplus
}
#endif

#endif
