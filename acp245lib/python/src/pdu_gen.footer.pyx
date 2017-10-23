class ParserError(Exception):
    def __init__(self, code, message=None):
        Exception.__init__(self, message or 'parser error: %x' % code)
        self.code = code

class MessageError(ParserError):
    pass

class InvalidArgumentError(MessageError):
    def __init__(self, message):
        MessageError.__init__(self, 0, message=message)

def msg_read(bin=None, hex=None):
    cdef e_buff buff
    cdef acp_msg msg
    cdef e_ret r
    cdef size_t sz
    cdef u8 *data 
    cdef u32 start

    if (not bin and not hex) or (bin and hex):
        raise ParserError(
            "Must provide a binary buffer OR a hex str as parameters "\
            "(bin='' or hex='')")
    if hex:
        bin = binascii.a2b_hex(hex)

    sz = len(bin)
    data = <u8*>malloc(sz * sizeof(u8))
    try:
        e_buff_wrap(&buff, data, sz)
        for b in bin:
            e_buff_write(&buff, <u8>ord(b))
        start = e_buff_read_remain(&buff)
        r = acp_msg_read(&buff, &msg);
        if r:
            raise MessageError(r)

        hdr = acp_hdr_to_py(&msg.hdr)

        py_msg = None
        if hdr.app_id == ACP_APP_ID_PROVISIONING:
            if hdr.type == ACP_MSG_TYPE_PROV_UPD:
                py_msg = acp_msg_prov_upd_to_py(hdr, &msg.data.prov_upd)
            elif hdr.type == ACP_MSG_TYPE_PROV_REPLY:
                py_msg = acp_msg_prov_reply_to_py(hdr, &msg.data.prov_reply)
        elif hdr.app_id == ACP_APP_ID_CONFIGURATION:
            if hdr.type == ACP_MSG_TYPE_CFG_UPD_245:
                py_msg = acp_msg_cfg_upd_245_to_py(hdr, &msg.data.cfg_upd_245)
            elif hdr.type == ACP_MSG_TYPE_CFG_REPLY:
                py_msg = acp_msg_cfg_reply_to_py(hdr, &msg.data.cfg_reply)
            elif hdr.type == ACP_MSG_TYPE_CFG_REPLY_245:
                py_msg = acp_msg_cfg_reply_245_to_py(hdr, &msg.data.cfg_reply_245)
            elif hdr.type == ACP_MSG_TYPE_CFG_ACT_245:
                py_msg = acp_msg_cfg_activation_to_py(hdr, &msg.data.cfg_activation)
        elif hdr.app_id == ACP_APP_ID_REMOTE_VEHICLE_FUNCTION:
            if hdr.type == ACP_MSG_TYPE_FUNC_CMD:
                py_msg = acp_msg_func_cmd_to_py(hdr, &msg.data.func_cmd)
            elif hdr.type == ACP_MSG_TYPE_FUNC_STATUS:
                py_msg = acp_msg_func_status_to_py(hdr, &msg.data.func_status)
        elif hdr.app_id == ACP_APP_ID_VEHICLE_TRACKING:
            if hdr.type == ACP_MSG_TYPE_TRACK_POS:
                py_msg = acp_msg_track_pos_to_py(hdr, &msg.data.track_pos)
            elif hdr.type == ACP_MSG_TYPE_TRACK_REPLY:
                py_msg = acp_msg_track_reply_to_py(hdr, &msg.data.track_reply)
            elif hdr.type == ACP_MSG_TYPE_TRACK_CMD:
                py_msg = acp_msg_track_cmd_to_py(hdr, &msg.data.track_cmd)
        elif hdr.app_id == ACP_APP_ID_ALARM:
            if hdr.type == ACP_MSG_TYPE_ALARM_NOTIF:
                py_msg = acp_msg_alarm_notif_to_py(hdr, &msg.data.alarm_notif)
            elif hdr.type == ACP_MSG_TYPE_ALARM_REPLY:
                py_msg = acp_msg_alarm_reply_to_py(hdr, &msg.data.alarm_reply)
            elif hdr.type == ACP_MSG_TYPE_ALARM_POS:
                py_msg = acp_msg_alarm_pos_to_py(hdr, &msg.data.alarm_pos)
            elif hdr.type == ACP_MSG_TYPE_ALARM_KA:
                py_msg = acp_msg_alarm_ka_to_py(hdr, &msg.data.alarm_ka)
            elif hdr.type == ACP_MSG_TYPE_ALARM_KA_REPLY:
                py_msg = acp_msg_alarm_ka_reply_to_py(hdr, &msg.data.alarm_ka_reply)
        if py_msg is None:
            raise MessageError(0,
                'unknown message: app=%d type=%d' % (hdr.app_id, hdr.type))
        return py_msg, start - e_buff_read_remain(&buff)
    finally:
        free(data)
        acp_msg_free(&msg)

def msg_write(msg):
    return msg.as_bytes()

def msg_sign(kt_str, iccid_str, date_str, msg_obj):
    cdef u32 kt_len
    cdef u8* kt = NULL
    cdef u32 iccid_len
    cdef u8* iccid = NULL
    cdef u32 date_len
    cdef u8* date = NULL
    cdef u32 msg_len
    cdef u8* msg = NULL
    cdef u8* ks = NULL
    cdef u8 ks_len = ACP_KEY_AUTH_KEY_LEN
    cdef object sign

    if not isinstance(msg_obj, CfgActivation):
        raise ValueError("Message to sign must be a CfgActivation message")

    # clear auth key
    vehicle_desc = msg_obj.vehicle_desc
    if vehicle_desc is not None:
        old_auth_key = msg_obj.vehicle_desc.auth_key
        msg_obj.vehicle_desc.auth_key = b'\x00'*8
    else:
        msg_obj.vehicle_desc = IEVehicleDesc(auth_key = b'\x00'*8)

    try:
        kt = str_tuple_to_u8_ptr(kt_str, &kt_len)
        if kt_len > 0xFF:
            raise ValueError("KT is too large")
        iccid = str_tuple_to_u8_ptr(iccid_str, &iccid_len)
        if iccid_len > 0xFF:
            raise ValueError("ICCID is too large")
        date = str_tuple_to_u8_ptr(date_str, &date_len)
        if date_len > 0xFF:
            raise ValueError("Date is too large")
        msg = str_tuple_to_u8_ptr(msg_obj.as_bytes(), &msg_len)
        if msg_len > 0xFFFF:
            raise ValueError("Message is too large")

        ks = <u8*>malloc(ks_len * sizeof(u8))
        c_pdu.acp_key_get(kt, <u8>(kt_len & 0xFF),
                    iccid, <u8>(iccid_len & 0xFF),
                    date, <u8>(date_len & 0xFF),
                    msg, <u16>(msg_len & 0xFFFF),
                    ks, ks_len)

        sign = u8_ptr_to_py_buff(ks, ks_len)
        return sign
    finally:
        free(kt)
        free(iccid)
        free(date)
        free(msg)
        free(ks)
        if vehicle_desc is not None:
            msg_obj.vehicle_desc.auth_key = old_auth_key
        else:
            msg_obj.vehicle_desc = None

# TODO remove.
class Parser(object):
    def msg_read(self, str_data):
        return msg_read(str_data)
    def msg_write(self, msg):
        return msg_write(msg)
