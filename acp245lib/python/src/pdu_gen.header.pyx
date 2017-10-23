cimport c_pdu_gen as c_pdu

def acp_init(self, char* license_file):
    return c_pdu.acp_init_opts(license_file)

import os
if u'E_ACP245_LICENSE' in os.environ:
    c_pdu.acp_init_opts(<char*>os.environ[u'E_ACP245_LICENSE'])
else:
    license = os.path.join(u'/',u'etc', u'acp245',u'license.sig')
    if os.path.isfile(license):
        c_pdu.acp_init_opts(<char*>license)
    else:
        c_pdu.acp_init_opts('license.sig')

import binascii
import re
import socket
from c_pdu_gen cimport \
        acp_msg_app_id, acp_msg_type, acp_msg_hdr_prio, \
        acp_msg_is_reply_codes, acp_msg_is_tcu_message, acp_msg_is_so_message, \
        acp_ie_any, acp_msg, acp_msg_free, acp_hdr,\
        acp_msg_write, acp_msg_read
from c_pdu_lib cimport *

# using except * is not the most performant option, but the safer
cdef void list_to_u8_ptr_f(object l, u8 *ptr, u32 len) except *:
    if not isinstance(l, list) and not isinstance(l, tuple) and not isinstance(l, str):
        raise ValueError("Field value must be a binary tuple, list or string.")
    for i from 0 <= i < len:
        ptr[i] = <u8>(l[i] & 0xFF)

cdef u8 *str_tuple_to_u8_ptr(object s, u32 *l) except *:
    if not s:
        l[0] = 0
        return NULL
    if not isinstance(s, str) and not isinstance(s, tuple):
        raise ValueError("Field value must be a binary tuple or string.")
        l[0] = 0
        return NULL
    cdef u8* p = <u8*>malloc(len(s) * sizeof(u8))
    l[0] = len(s)
    for i from 0 <= i < l[0]:
        if isinstance(s,str):
            p[i] = <u8>(ord(s[i]) & 0xFF)
        else:
            p[i] = <u8>(s[i] & 0xFF)
    return p

cdef object u8_ptr_to_tuple(u8 *ptr, u32 len):
    if ptr == NULL:
        return None
    str = []
    for i from 0 <= i < len:
        str.append(ptr[i])
    return tuple(str)

cdef object u8_ptr_to_py_buff(u8 *ptr, u32 len):
    if ptr == NULL:
        return None
    str = b''
    for i from 0 <= i < len:
        str += chr(ptr[i])
    return str

cdef object str_or_none(char *s):
    if s == NULL:
        return None
    else:
        return s

cdef object semi(object num):
    n = b''
    for x in range(len(num)/2):
        n += chr(int(num[(x*2)+1]) << 4 | int(num[x*2]))
    if len(num) % 2 != 0:
        n += chr(0xF0 | int(num[-1]))
    return n

cdef object u32_ntoa(u32 ip):
    if ip == 0:
        return None
    s = socket.inet_ntoa(b''.join([chr(x) for x in (
        ((ip >> 24) & 0xFF),
        ((ip >> 16) & 0xFF),
        ((ip >> 8) & 0xFF),
        ((ip) & 0xFF)
        )]))
    return s

cdef u32 u32_aton(object s) except *:
    if isinstance(s, int):
        return s
    s = socket.inet_aton(s)
    return (((ord(s[0])&0xFF) << 24) |
            ((ord(s[1])&0xFF) << 16) |
            ((ord(s[2])&0xFF) << 8) |
            ((ord(s[3])&0xFF))
           )

# credit to
# http://stackoverflow.com/questions/92438/stripping-non-printable-characters-from-a-string-in-python
__control_chars = u''.join([unichr(x) for x in range(0,32) + range(127,160)])
__control_char_re = re.compile(u'[%s]' % re.escape(__control_chars))
def __printable(s):
    return not __control_char_re.match(s)

def __to_py(member, class_to_py):
    if member is None:
        v = None
    elif isinstance(member, Element) \
            or isinstance(member, Header) \
            or isinstance(member, Message):
        v = class_to_py(member)
    elif isinstance(member, list) or isinstance(member, tuple):
        l = []
        for x in member:
            l.append(__to_py(x, class_to_py))
        v = l
    elif isinstance(member, str):
        try:
            v = unicode(member)
        except:
            v = u'0x%s' % binascii.b2a_hex(member)
    else:
        v = member
    return v

def __as_dict(element):
    d = {}
    for name in [x for x in dir(element) if not x.startswith('_')]:
        member = getattr(element, name)
        if not callable(member):
            d[unicode(name)] = __to_py(member, __as_dict)
    return d

def __as_tree(element):

    fields = []
    for name in element.get_fields():
        member = getattr(element, name)
        if not callable(member):
            fields.append((unicode(name), __to_py(member, __as_tree)))
    d = {
        u'type': unicode(element.__class__.__name__),
        u'fields': tuple(fields)
    }
    return d

def __as_dict_str(element):
    d = {}
    for name in element.get_fields():
        member = getattr(element, name)
        if not callable(member):
            d[name] = member
    return d

cdef class Element:
    """A base class for ACP245 information elements."""
    def as_dict(self):
        return __as_dict(self)

    def as_tree(self):
        """
        Returns the header as a list of tuples as:
            [(<field_name>,<field_value>),...]
        With fields ordered according to the protocol ordering.
        """
        return __as_tree(self)

    def __str__(self):
        return repr(self)

    def __repr__(self):
        d = __as_dict_str(self)
        return '%s(%s)' % (
                self.__class__.__name__,
                ','.join(['%s=%s' % (k, repr(v)) for k, v in d.items()])
        )

cdef class IEAny(Element):
    """A generic ACP245 information element."""
    cdef public u8 id
    cdef public object data
    def __init__(self, u8 id, object data):
        self.id = id
        self.data = data

    def __str__(self):
        if isinstance(self.data, tuple):
            return str(''.join(map(chr, self.data)))
        else:
            return str(self.data)

    cdef void _to_struct(self, acp_ie_any *ie) except *:
        cdef u32 len
        ie.id = self.id
        ie.present = 1
        if self.data:
            if self.id == c_pdu.ACP_IE_ISO_8859_1 or self.id == c_pdu.ACP_IE_PACKED_DEC:
                ie.data.str = strdup(<char*>self.data)
            else:
                ie.data.bin = str_tuple_to_u8_ptr(self.data, &len)
                if (len > 0xFFFF):
                    raise ValueError('Unsupported data length')
                ie.len = <u16> len
cdef object acp_ie_any_to_py(acp_ie_any *ie):
    if not ie.present:
        return None
    if c_pdu.ACP_IE_ISO_8859_1 == ie.id:
        return str_or_none(<char*>ie.data.str)
    elif c_pdu.ACP_IE_PACKED_DEC == ie.id:
        s = str_or_none(<char*>ie.data.str)
        if s is not None:
            s = int(s)
        return s
    else:
        return IEAny(ie.id, u8_ptr_to_tuple(ie.data.bin, ie.len))
cdef void to_acp_ie_any(acp_ie_any *ie, object o):
    if isinstance(o, str):
        ie.present = 1
        ie.id = c_pdu.ACP_IE_ISO_8859_1
        ie.data.str = strdup(<char*>o)
    elif isinstance(o, int):
        ie.present = 1
        ie.id = c_pdu.ACP_IE_PACKED_DEC
        s = str(o)
        ie.data.str = strdup(<char*>s)
    elif isinstance(o, IEAny):
        (<IEAny>o)._to_struct(ie)
    else:
        raise ValueError('Invalid IE value: %s' % o)

cdef class Header:
    """An ACP245 message header."""
    cdef public u8 app_id
    cdef public bool test
    cdef public u8 type
    cdef public u8 version
    cdef public u8 msg_ctrl
    cdef public u8 msg_prio

    def __init__(self,
                 u8 app_id=0,
                 u8 type=0,
                 bool test=0,
                 u8 version=0,
                 u8 msg_ctrl=0,
                 u8 msg_prio=0
                ):
        self.app_id = app_id
        self.type = type
        self.version = version
        self.msg_ctrl = msg_ctrl
        self.msg_prio = msg_prio
        self.test = test

    def as_dict(self):
        """Returns the header as a dictionary of field_name->field_value."""
        return __as_dict(self)

    def as_tree(self):
        """
        Returns the header as a list of tuples as:
            [(<field_name>,<field_value>),...]
        With fields ordered according to the protocol ordering.
        """
        return __as_tree(self)

    def get_fields(self):
        """Returns the header field names orderer according to the protocol
        ordering."""
        return 'app_id', 'test', 'type', 'version', 'msg_ctrl', 'msg_prio'

    def __str__(self):
        return repr(self)

    def __repr__(self):
        d = __as_dict_str(self)
        return '%s(%s)' % (
                self.__class__.__name__,
                ','.join(['%s=%s' % (k, repr(v)) for k, v in d.items()])
        )

    cdef void _to_struct(self, acp_msg *msg) except *:
        msg.hdr.app_id = <acp_msg_app_id>self.app_id
        msg.hdr.type = <acp_msg_type>self.type
        msg.hdr.version = self.version
        msg.hdr.msg_ctrl = self.msg_ctrl
        msg.hdr.msg_prio = <acp_msg_hdr_prio>self.msg_prio
        msg.hdr.test = self.test

cdef Header acp_hdr_to_py(acp_hdr *hdr):
    return Header(
            hdr.app_id,
            hdr.type,
            hdr.test,
            hdr.version,
            hdr.msg_ctrl,
            hdr.msg_prio)

cdef class Message:
    """An ACP245 message."""
    cdef public Header header
    def __init__(self, Header header=None):
        if header is None:
            header = Header(self.__class__.app_id, self.__class__.type)
        header.app_id = self.__class__.app_id
        header.type = self.__class__.type
        self.header = header

    def is_valid_tcu_msg(self):
        """Returns True if this message can be sent from a TCU. The method checks
        the message application ID and type, and also that the message includes
        a vehicle description with ICCID if available (as required by section
        [ACP245] 14.4)"""
        return (acp_msg_is_tcu_message(self.app_id, self.type) and
                (not hasattr(self, 'vehicle_desc') or
                 (self.vehicle_desc is not None and
                  self.vehicle_desc.iccid is not None) or
                 isinstance(self, AlarmKA))
               )

    def is_valid_tcu_first_msg(self):
        """Returns True if this message is a valid first message from a TCU. The
        first message must be a TCU message, and have a non zero version an
        vehicle descriptor element, as described on [ACP245] section 14.3."""
        return (acp_msg_is_tcu_message(self.app_id, self.type)
                and hasattr(self, 'version')
                and self.version is not None
                and self.version.present
                and hasattr(self, 'vehicle_desc') and
                self.vehicle_desc is not None and
                self.vehicle_desc.iccid is not None)

    def is_valid_so_msg(self):
        """Returns True if this message can be sent from a SO. The method checks
        the message application ID and type, and also that the message includes
        a vehicle description with ICCID if available (as required by section
        [ACP245] 14.4)"""
        return (acp_msg_is_so_message(self.app_id, self.type) and
                (not hasattr(self, 'vehicle_desc') or
                 (self.vehicle_desc is not None and
                  self.vehicle_desc.iccid is not None) or
                 isinstance(self, AlarmKAReply))
               )


    def expect_reply(self):
        """Sets the response expected flag of the message header."""
        self.header.msg_ctrl |= c_pdu.ACP_HDR_MSG_CTRL_RESP_EXP
        return self

    def is_reply(self, msg):
        """Returns True if this message is a reply for msg."""
        return acp_msg_is_reply_codes(msg.app_id, msg.type, self.app_id, self.type)

    def app_id(self):
        return self.header.app_id
    app_id = property(app_id)
    def type(self):
        return self.header.type
    type = property(type)

    def as_bytes(self):
        """Returns the message in it's binary representation."""
        cdef e_buff buff
        cdef acp_msg msg
        cdef e_ret r
        cdef u8 *data = <u8*>malloc(1024 * sizeof(u8))
        try:
            memset(&msg, 0, sizeof(msg))
            self.header._to_struct(&msg)
            self._to_struct(&msg)
            e_buff_wrap(&buff, data, 1024)
            r = acp_msg_write(&buff, &msg)
            if r:
                raise Exception('acp_msg_write: %x' % r)
            return u8_ptr_to_py_buff(data, e_buff_read_remain(&buff))
        finally:
            free(data)
            acp_msg_free(&msg)

    def as_bytes_hex(self):
        """Returns the message in it's hexadecimal representation."""
        return binascii.b2a_hex(self.as_bytes()).upper()

    def as_dict(self):
        """Returns the message as a dictionary of field_name->field_value."""
        return __as_dict(self)

    def as_tree(self):
        """
        Returns the header as a list of tuples as:
            [(<field_name>,<field_value>),...]
        With fields ordered according to the protocol ordering.
        """
        tree = __as_tree(self)
        tree[u'fields'] = ((u'header', self.header.as_tree()),) + tree[u'fields']
        return tree

    def as_at_pdu(self, number, ton=b'\x81', msg_ref=b'\x00'):
        """
        Represent the message in a suitable format to be sent using a standards
        compliant GSM modem.
        The output of this command can be used to send a binary SMS with the
        message. For example:
            AT+CMGS=<length of output - 1>
            > <copy output in hex here><ctrl-Z>
        ATTENTION: This method will probaby be deprecated on later version. Is
        included to perform initial ACP245 tests.
        """
        semi_num = semi(number)
        bytes = self.as_bytes()
        hdr = b''.join((
                b'\x00',             # SMSC info length, not included in PDU len
                b'\x11',             # First octet of SMS-SUBMIT message
                                    # 0x11 = message type SMS SUBMIT, validity
                                    # period present and relative
                msg_ref,            # Message reference
                chr(len(number)),   # Length of the phone number (11)
                ton,                # Type of number (0x81 = international)
                semi_num,           # Telephone number
                b'\x00',             # TP-PID Protocol ID.
                b'\x04',             # TP-DCS data coding scheme. 0x4=8 bit data
                b'\xAA',             # Validity period
                chr(len(bytes)),    # TP-User-Data-Length. Length of the message
        ))
        return hdr + bytes

    def as_at_pdu_hex(self, number, ton=b'\x81', msg_ref=b'\x00'):
        """
        See as_at_pdu documentation.
        ATTENTION: This method will probaby be deprecated on later version. Is
        included to perform initial ACP245 tests.
        """
        return binascii.b2a_hex(self.as_at_pdu(number, ton=ton, msg_ref=msg_ref))

    def set_reply_expected(self, expected):
        """Sets the response expected flag of the message header."""
        if expected:
            self.header.msg_ctrl |= c_pdu.ACP_HDR_MSG_CTRL_RESP_EXP
        else:
            self.header.msg_ctrl &= ~c_pdu.ACP_HDR_MSG_CTRL_RESP_EXP

    def is_reply_expected(self):
        """Returns True if a reply is expected for this message."""
        return (self.header.msg_ctrl & c_pdu.ACP_HDR_MSG_CTRL_RESP_EXP) != 0

    def __str__(self):
        return self.__class__.__name__

    def __repr__(self):
        d = __as_dict_str(self)
        return u'%s(%s)' % (
                self.__class__.__name__,
                ','.join([u'%s=%s' % (k, repr(v)) for k, v in d.items() \
                         if k not in (u'app_id',u'type')])
        )

    cdef void _to_struct(self, acp_msg *msg) except *:
        pass
