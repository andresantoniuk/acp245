"""
Common field argument descriptors.

This module provides argument descriptors to map a set of arguments to an ACP245
PDU and viceversa. It's used mainly by the gateway module, but can also be used
by other interfaces which accepts arguments in the form key->value that should
be mapped to an ACP245 message.

See the acpyd.args module for additional information.
"""
from acp245.pdu import *
from acpyd.args import *

FLATTENERS = {
    CfgUpd245: (
    ('hdr_msg_ctrl',            Int8('header.msg_ctrl',
                                    values={
                                        'RESP_EXP': ACP_HDR_MSG_CTRL_RESP_EXP,
                                    })),
    ('hdr_test_flag',           Int8('header.test')),
    ('hdr_version',             Int8('header.version')),
    ('car_manufacturer_id',     Int8('version.car_manufacturer')),
    ('tcu_manufacturer_id',     Int8('version.tcu_manufacturer')),
    ('major_hardware_release',  Int8('version.major_hard_rel')),
    ('major_software_release',  Int8('version.major_soft_rel')),
    ('target_application_id',   Int8('target_app_id')),

    ('action',                  Int8('appl_flg',
                                    values={
                                        'QUERY':    ACP_MSG_PROV_NO_CHANGE,
                                        'ACTIVATE': ACP_MSG_PROV_ACTIVATE,
                                        'DEACTIVATE':ACP_MSG_PROV_DEACTIVATE,
                                        'CHANGE':   ACP_MSG_PROV_CHANGE,
                                    }
                                )),
    ('start_time',          Time('start_time')),
    ('end_time',            Time('end_time')),
    ('grace_time',          Time('grace_time')),
    ('device_id',           Int8('tcu_desc.device_id')),
    ('version_id',          Int8('tcu_desc.version.id')),
    ('vin_number',          String('vehicle_desc.vin')),
    ('tcu_serial_number',   String('vehicle_desc.tcu_serial')),
    ('imei_number',         String('vehicle_desc.imei')),
    ('sim_card_id',         String('vehicle_desc.iccid')),
    ('authorization_key',   BinHex('vehicle_desc.auth_key')),
    ('p',                   List('tcu_data.items', IETCUDataItem, (
                                ('cod', Int8('type')),
                                ('val', BinHex('data'))
                                )
                            ))

    ),
    CfgReply: (
    ('hdr_msg_ctrl',            Int8('header.msg_ctrl',
                                    values={
                                        'RESP_EXP': ACP_HDR_MSG_CTRL_RESP_EXP,
                                    })),
    ('hdr_test_flag',           Int8('header.test')),
    ('hdr_version',             Int8('header.version')),
    ('car_manufacturer_id',     Int8('version.car_manufacturer')),
    ('tcu_manufacturer_id',     Int8('version.tcu_manufacturer')),
    ('major_hardware_release',  Int8('version.major_hard_rel')),
    ('major_software_release',  Int8('version.major_soft_rel')),

    ('target_application_id',   Int8('target_app_id')),

    ('vin_number',              String('vehicle_desc.vin')),
    ('tcu_serial_number',       String('vehicle_desc.tcu_serial')),
    ('imei_number',             String('vehicle_desc.imei')),
    ('sim_card_id',             String('vehicle_desc.iccid')),
    ('authorization_key',       BinHex('vehicle_desc.auth_key')),

    ('error',                   Int8('error.code')),
    ),
    CfgReply245: (
    ('hdr_msg_ctrl',            Int8('header.msg_ctrl',
                                    values={
                                        'RESP_EXP': ACP_HDR_MSG_CTRL_RESP_EXP,
                                    })),
    ('hdr_test_flag',           Int8('header.test')),
    ('hdr_version',             Int8('header.version')),
    ('car_manufacturer_id',     Int8('version.car_manufacturer')),
    ('tcu_manufacturer_id',     Int8('version.tcu_manufacturer')),
    ('major_hardware_release',  Int8('version.major_hard_rel')),
    ('major_software_release',  Int8('version.major_soft_rel')),

    ('target_application_id',   Int8('target_app_id')),

    ('vin_number',              String('vehicle_desc.vin')),
    ('tcu_serial_number',       String('vehicle_desc.tcu_serial')),
    ('imei_number',             String('vehicle_desc.imei')),
    ('sim_card_id',             String('vehicle_desc.iccid')),
    ('authorization_key',       BinHex('vehicle_desc.auth_key')),

    ('errors',                  List('error(IETCUDataError).items', IETCUDataErrorItem, (
                                    ('type', Int8('type')),
                                    ('data', BinHex('data')),
                                    ('code', Int8('error.code'))
                                    )
                                )),
    ),
    FuncCmd: (
    ('hdr_msg_ctrl',            Int8('header.msg_ctrl',
                                    values={
                                        'RESP_EXP': ACP_HDR_MSG_CTRL_RESP_EXP,
                                    })),
    ('hdr_test_flag',           Int8('header.test')),
    ('hdr_version',             Int8('header.version')),
    ('car_manufacturer_id',     Int8('version.car_manufacturer')),
    ('tcu_manufacturer_id',     Int8('version.tcu_manufacturer')),
    ('major_hardware_release',  Int8('version.major_hard_rel')),
    ('major_software_release',  Int8('version.major_soft_rel')),
    ('entity_id',               Int8('ctrl_func.entity_id')),
    ('transmit_unit',           Int8('ctrl_func.transmit_unit',
                                   values={
                                       'SECOND' :ACP_EL_TIME_UNIT_SECOND,
                                       'MINUTE' :ACP_EL_TIME_UNIT_MINUTE,
                                       'HOUR'   :ACP_EL_TIME_UNIT_HOUR,
                                       'ONE_MORE_TIME'  :ACP_EL_TIME_UNIT_ONEMORE,
                                       'ONLY_ONE'   :ACP_EL_TIME_UNIT_ONLYONE,
                                   }
                                )),
    ('transmit_interval',       Int8('ctrl_func.transmit_interval')),


    ('function_command',        Int8('func_cmd.cmd',
                                   values={
                                       'PERMIT' :ACP_FUNC_CMD_PERMIT,
                                       'REJECT' :ACP_FUNC_CMD_REJECT,
                                       'ENABLE' :ACP_FUNC_CMD_ENABLE,
                                       'DISABLE':   ACP_FUNC_CMD_DISABLE,
                                       'REQUEST':   ACP_FUNC_CMD_REQUEST,
                                    }
                                )),

    ('raw_data',            BinHex('func_cmd.data')),

    ('vin_number',          String('vehicle_desc.vin')),
    ('tcu_serial_number',   String('vehicle_desc.tcu_serial')),
    ('imei_number',         String('vehicle_desc.imei')),
    ('sim_card_id',         String('vehicle_desc.iccid')),
    ('authorization_key',   BinHex('vehicle_desc.auth_key')),

    ),
    FuncStatus: (
    ('hdr_msg_ctrl',            Int8('header.msg_ctrl',
                                    values={
                                        'RESP_EXP': ACP_HDR_MSG_CTRL_RESP_EXP,
                                    })),
    ('hdr_test_flag',           Int8('header.test')),
    ('hdr_version',             Int8('header.version')),
    ('error',                   Int8('error.code')),

    ('car_manufacturer_id',     Int8('version.car_manufacturer')),
    ('tcu_manufacturer_id',     Int8('version.tcu_manufacturer')),
    ('major_hardware_release',  Int8('version.major_hard_rel')),
    ('major_software_release',  Int8('version.major_soft_rel')),
    ('vin_number',              String('vehicle_desc.vin')),
    ('tcu_serial_number',       String('vehicle_desc.tcu_serial')),
    ('imei_number',             String('vehicle_desc.imei')),
    ('sim_card_id',             String('vehicle_desc.iccid')),
    ('authorization_key',       BinHex('vehicle_desc.auth_key')),
    ('entity_id',               Int8('ctrl_func.entity_id')),
    ('transmit_unit',           Int8('ctrl_func.transmit_unit',
                                   values={
                                       'SECOND' :ACP_EL_TIME_UNIT_SECOND,
                                       'MINUTE' :ACP_EL_TIME_UNIT_MINUTE,
                                       'HOUR'   :ACP_EL_TIME_UNIT_HOUR,
                                       'ONE_MORE_TIME'  :ACP_EL_TIME_UNIT_ONEMORE,
                                       'ONLY_ONE'   :ACP_EL_TIME_UNIT_ONLYONE,
                                   }
                                )),

    ('transmit_interval',       Int8('ctrl_func.transmit_interval')),

    ('function_status',         Int8('func_status(IEFuncCmd).cmd',
                                  values={
                                      'PERMITTED'   :ACP_FUNC_STATE_PERMITTED,
                                      'REJECTED'    :ACP_FUNC_STATE_REJECTED,
                                      'ENABLED'     :ACP_FUNC_STATE_ENABLED,
                                      'DISABLED'    :ACP_FUNC_STATE_DISABLED,
                                      'COMPLETED'   :ACP_FUNC_STATE_COMPLETED,
                                    }
                                )),
    ('raw_data',            BinHex('func_status(IEFuncCmd).data')),
    ),
    ProvUpd: (
    ('hdr_msg_ctrl',            Int8('header.msg_ctrl',
                                    values={
                                        'RESP_EXP': ACP_HDR_MSG_CTRL_RESP_EXP,
                                    })),
    ('hdr_test_flag',           Int8('header.test')),
    ('hdr_version',             Int8('header.version')),
    ('car_manufacturer_id',     Int8('version.car_manufacturer')),
    ('tcu_manufacturer_id',     Int8('version.tcu_manufacturer')),
    ('major_hardware_release',  Int8('version.major_hard_rel')),
    ('major_software_release',  Int8('version.major_soft_rel')),
    ('target_application_id',   Int8('target_app_id')),

    ('action',                  Int8('appl_flg',
                                    values={
                                        'QUERY':    ACP_MSG_PROV_NO_CHANGE,
                                        'ACTIVATE': ACP_MSG_PROV_ACTIVATE,
                                        'DEACTIVATE':ACP_MSG_PROV_DEACTIVATE,
                                        'CHANGE':   ACP_MSG_PROV_CHANGE,
                                    }
                                )),
    ('start_time',          Time('start_time')),
    ('end_time',            Time('end_time')),
    ('device_id',           Int8('tcu_desc.device_id')),
    ('version_id',          Int8('tcu_desc.version.id')),
    ('vin_number',          String('vehicle_desc.vin')),
    ('tcu_serial_number',   String('vehicle_desc.tcu_serial')),
    ('imei_number',         String('vehicle_desc.imei')),
    ('sim_card_id',         String('vehicle_desc.iccid')),
    ('authorization_key',   BinHex('vehicle_desc.auth_key')),
    ),
    ProvReply: (
    ('hdr_msg_ctrl',            Int8('header.msg_ctrl',
                                    values={
                                        'RESP_EXP': ACP_HDR_MSG_CTRL_RESP_EXP,
                                    })),
    ('hdr_test_flag',           Int8('header.test')),
    ('hdr_version',             Int8('header.version')),
    ('error',                   Int8('error.code')),
    ('target_application_id',   Int8('target_app_id')),
    ('action',                  Int8('appl_flg',
                                    values={
                                        'QUERY':    ACP_MSG_PROV_NO_CHANGE,
                                        'ACTIVATE': ACP_MSG_PROV_ACTIVATE,
                                        'DEACTIVATE':ACP_MSG_PROV_DEACTIVATE,
                                        'CHANGE':   ACP_MSG_PROV_CHANGE,
                                    }
                                )),
    ('vin_number',              String('vehicle_desc.vin')),
    ('tcu_serial_number',       String('vehicle_desc.tcu_serial')),
    ('imei_number',             String('vehicle_desc.imei')),
    ('sim_card_id',             String('vehicle_desc.iccid')),
    ('authorization_key',       BinHex('vehicle_desc.auth_key')),
    ),
    TrackCmd: (
    ('hdr_msg_ctrl',            Int8('header.msg_ctrl',
                                    values={
                                        'RESP_EXP': ACP_HDR_MSG_CTRL_RESP_EXP,
                                    })),
    ('hdr_test_flag',           Int8('header.test')),
    ('hdr_version',             Int8('header.version')),
    ('car_manufacturer_id',     Int8('version.car_manufacturer')),
    ('tcu_manufacturer_id',     Int8('version.tcu_manufacturer')),
    ('major_hardware_release',  Int8('version.major_hard_rel')),
    ('major_software_release',  Int8('version.major_soft_rel')),
    ('entity_id',               Int8('ctrl_func.entity_id')),
    ('transmit_unit',           Int8('ctrl_func.transmit_unit',
                                    values={
                                       'SECOND':    ACP_EL_TIME_UNIT_SECOND,
                                       'MINUTE':    ACP_EL_TIME_UNIT_MINUTE,
                                       'HOUR':  ACP_EL_TIME_UNIT_HOUR,
                                       'ONE_MORE_TIME': ACP_EL_TIME_UNIT_ONEMORE,
                                       'ONLY_ONE':  ACP_EL_TIME_UNIT_ONLYONE,
                                   }
                                )),

    ('transmit_interval',       Int8('ctrl_func.transmit_interval')),

    ('function_command',        Int8('func_cmd.cmd',
                                   values={
                                       'PERMIT':    ACP_FUNC_CMD_PERMIT,
                                       'REJECT':    ACP_FUNC_CMD_REJECT,
                                       'ENABLE':    ACP_FUNC_CMD_ENABLE,
                                       'DISABLE':   ACP_FUNC_CMD_DISABLE,
                                       'REQUEST':   ACP_FUNC_CMD_REQUEST,
                                   }
                                )),
    ('raw_data',            BinHex('func_cmd.data')),

    ('vin_number',          String('vehicle_desc.vin')),
    ('tcu_serial_number',   String('vehicle_desc.tcu_serial')),
    ('imei_number',         String('vehicle_desc.imei')),
    ('sim_card_id',         String('vehicle_desc.iccid')),
    ('authorization_key',   BinHex('vehicle_desc.auth_key')),

    ),
    TrackReply: (
    ('hdr_msg_ctrl',            Int8('header.msg_ctrl',
                                    values={
                                        'RESP_EXP': ACP_HDR_MSG_CTRL_RESP_EXP,
                                    })),
    ('hdr_test_flag',           Int8('header.test')),
    ('hdr_version',             Int8('header.version')),
    ('car_manufacturer_id',     Int8('version.car_manufacturer')),
    ('tcu_manufacturer_id',     Int8('version.tcu_manufacturer')),
    ('major_hardware_release',  Int8('version.major_hard_rel')),
    ('major_software_release',  Int8('version.major_soft_rel')),

    ('error',                   Int8('error.code')),

    ('confirmation',            Int8('confirmation',
                                   values={
                                       'ACCEPTED'
                                       :ACP_MSG_TRACK_CONFIRM_ACCEPTED,
                                       'TURN_SPEAKER'
                                       :ACP_MSG_TRACK_CONFIRM_TURN_SPEAKER,
                                       'START_TRACKING':
                                       ACP_MSG_TRACK_CONFIRM_PROCESS_START,
                                   }
                                )),
    ('transmit_unit',           Int8('transmit_unit',
                                   values={
                                       'SECOND' :ACP_EL_TIME_UNIT_SECOND,
                                       'MINUTE' :ACP_EL_TIME_UNIT_MINUTE,
                                       'HOUR'   :ACP_EL_TIME_UNIT_HOUR,
                                       'ONE_MORE_TIME'
                                       :ACP_EL_TIME_UNIT_ONEMORE,
                                       'ONLY_ONE'   :ACP_EL_TIME_UNIT_ONLYONE,
                                   }
                                )),
    ('ecall_update',            Int8('ctrl_flg',
                                  values={
                                      'CANCEL_ALARM'
                                      :ACP_MSG_TRACK_CTRL_CANCEL_ALARM,
                                      'RESEND_REQUEST'
                                      :ACP_MSG_TRACK_CTRL_RE_SEND_REQ,
                                      'DISABLE_VOICE_CALL'
                                      :ACP_MSG_TRACK_CTRL_DISABLE_VOICE,
                                  }
                                )),
    ),
    TrackPos: (
    ('hdr_msg_ctrl',            Int8('header.msg_ctrl',
                                    values={
                                        'RESP_EXP': ACP_HDR_MSG_CTRL_RESP_EXP,
                                    })),
    ('hdr_test_flag',           Int8('header.test')),
    ('hdr_version',             Int8('header.version')),
    ('car_manufacturer_id',     Int8('version.car_manufacturer')),
    ('tcu_manufacturer_id',     Int8('version.tcu_manufacturer')),
    ('major_hardware_release',  Int8('version.major_hard_rel')),
    ('major_software_release',  Int8('version.major_soft_rel')),
    ('timestamp',               Time('timestamp')),
    ('curr_area',               Int8('location.curr_gps(IEGPSRawData).area_type')),
    ('curr_location',           Int8('location.curr_gps(IEGPSRawData).location_type')),
    ('curr_lat',                Int32('location.curr_gps(IEGPSRawData).lat')),
    ('curr_lon',                Int32('location.curr_gps(IEGPSRawData).lon')),
    ('curr_alt',                Int16('location.curr_gps(IEGPSRawData).alt')),
    ('curr_velocity',           Int8('location.curr_gps(IEGPSRawData).velocity')),
    ('curr_heading',            Int8('location.curr_gps(IEGPSRawData).heading')),
    ('curr_pos_uncert',         Int8('location.curr_gps(IEGPSRawData).pos_uncert')),
    ('curr_sats',               List('location.curr_gps(IEGPSRawData).satellites', int)),

    ('vin_number',          String('vehicle_desc.vin')),
    ('tcu_serial_number',   String('vehicle_desc.tcu_serial')),
    ('imei_number',         String('vehicle_desc.imei')),
    ('sim_card_id',         String('vehicle_desc.iccid')),
    ('authorization_key',   BinHex('vehicle_desc.auth_key')),

    ('sources',             List('breakdown_status.source', int)),
    ('sensor',              Int8('breakdown_status.sensor')),
    ('data',                BinHex('breakdown_status.data')),
    ('info_type',           Int8('info_type.type')),
    ('info_data',           BinHex('info_type.data')),
    ),
    AlarmKA: (
    ('hdr_msg_ctrl',            Int8('header.msg_ctrl',
                                    values={
                                        'RESP_EXP': ACP_HDR_MSG_CTRL_RESP_EXP,
                                    })),
    ('hdr_test_flag',       Int8('header.test')),
    ('hdr_version',         Int8('header.version')),
    ('vin_number',          String('vehicle_desc.vin')),
    ('tcu_serial_number',   String('vehicle_desc.tcu_serial')),
    ('imei_number',         String('vehicle_desc.imei')),
    ('sim_card_id',         String('vehicle_desc.iccid')),
    ('authorization_key',   BinHex('vehicle_desc.auth_key')),
    ),
    AlarmNotif: (
    ('hdr_msg_ctrl',            Int8('header.msg_ctrl',
                                    values={
                                        'RESP_EXP': ACP_HDR_MSG_CTRL_RESP_EXP,
                                    })),
    ('hdr_test_flag',           Int8('header.test')),
    ('hdr_version',             Int8('header.version')),
    ('car_manufacturer_id',     Int8('version.car_manufacturer')),
    ('tcu_manufacturer_id',     Int8('version.tcu_manufacturer')),
    ('major_hardware_release',  Int8('version.major_hard_rel')),
    ('major_software_release',  Int8('version.major_soft_rel')),
    ('timestamp',               Time('timestamp')),
    ('curr_area',               Int8('location.curr_gps(IEGPSRawData).area_type')),
    ('curr_location',           Int8('location.curr_gps(IEGPSRawData).location_type')),
    ('curr_lat',                Int32('location.curr_gps(IEGPSRawData).lat')),
    ('curr_lon',                Int32('location.curr_gps(IEGPSRawData).lon')),
    ('curr_alt',                Int16('location.curr_gps(IEGPSRawData).alt')),
    ('curr_velocity',           Int8('location.curr_gps(IEGPSRawData).velocity')),
    ('curr_heading',            Int8('location.curr_gps(IEGPSRawData).heading')),
    ('curr_pos_uncert',         Int8('location.curr_gps(IEGPSRawData).pos_uncert')),
    ('curr_sats',               List('location.curr_gps(IEGPSRawData).satellites', int)),
    ('vin_number',          String('vehicle_desc.vin')),
    ('tcu_serial_number',   String('vehicle_desc.tcu_serial')),
    ('imei_number',         String('vehicle_desc.imei')),
    ('sim_card_id',         String('vehicle_desc.iccid')),
    ('authorization_key',   BinHex('vehicle_desc.auth_key')),
    ('sources',             List('breakdown_status.source', int)),
    ('sensor',              Int8('breakdown_status.sensor')),
    ('data',                BinHex('breakdown_status.data')),
    ('info_type',           Int8('info_type.type')),
    ('info_data',           BinHex('info_type.data')),
    ),
    AlarmReply: (
    ('hdr_msg_ctrl',            Int8('header.msg_ctrl',
                                    values={
                                        'RESP_EXP': ACP_HDR_MSG_CTRL_RESP_EXP,
                                    })),
    ('hdr_test_flag',           Int8('header.test')),
    ('hdr_version',             Int8('header.version')),
    ('car_manufacturer_id',     Int8('version.car_manufacturer')),
    ('tcu_manufacturer_id',     Int8('version.tcu_manufacturer')),
    ('major_hardware_release',  Int8('version.major_hard_rel')),
    ('major_software_release',  Int8('version.major_soft_rel')),

    ('error',                   Int8('error.code')),

    ('confirmation',            Int8('confirmation',
                                   values={
                                       'ACCEPTED'
                                       :ACP_MSG_TRACK_CONFIRM_ACCEPTED,
                                       'TURN_SPEAKER'
                                       :ACP_MSG_TRACK_CONFIRM_TURN_SPEAKER,
                                       'START_TRACKING':
                                       ACP_MSG_TRACK_CONFIRM_PROCESS_START,
                                   }
                                )),
    ('transmit_unit',           Int8('transmit_unit',
                                   values={
                                       'SECOND' :ACP_EL_TIME_UNIT_SECOND,
                                       'MINUTE' :ACP_EL_TIME_UNIT_MINUTE,
                                       'HOUR'   :ACP_EL_TIME_UNIT_HOUR,
                                       'ONE_MORE_TIME'
                                       :ACP_EL_TIME_UNIT_ONEMORE,
                                       'ONLY_ONE'   :ACP_EL_TIME_UNIT_ONLYONE,
                                   }
                                )),
    ('ecall_update',            Int8('ctrl_flg',
                                  values={
                                      'CANCEL_ALARM'
                                      :ACP_MSG_TRACK_CTRL_CANCEL_ALARM,
                                      'RESEND_REQUEST'
                                      :ACP_MSG_TRACK_CTRL_RE_SEND_REQ,
                                      'DISABLE_VOICE_CALL'
                                      :ACP_MSG_TRACK_CTRL_DISABLE_VOICE,
                                  }
                                )),
    ),
}
