import inspect
from unittest import TestCase
import binascii
from acp245 import pdu
from acp245 import log
import random
import sys
def _to_bin(hex_list):
    return binascii.a2b_hex(''.join('%.2X' % x for x in hex_list))

THRASH_CNT = 10

class PduTest(TestCase):
    def tearDown(self):
        log.setlevel(0)

    def test_key(self):
        kt = _to_bin([
            0x6A,0x72,0x39,0x53,0x1C,0xD8,0x58,0x64,0x23,0xFE,0xA4,0xEB,0xA3,0x82,0x1C,0xED,0xF5,0xCC,0xB6,0x25
        ])
        ks = _to_bin([
            0x4D, 0x0D, 0xF4, 0x41, 0x9F, 0x06, 0xB3, 0x85
        ])
        iccid = _to_bin([
            # ICCID == "1234567890123456789"
            0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,
        ])
        date = _to_bin([
            # DATE = 20080102
            0x07, 0xD8, 0x01, 0x02
        ])
        activation_msg = _to_bin([
            # header
            0x02,
            0x0A, # configuration, activation
            0x00,
            0x66, # length 102

            # apn_cfg
            0x1A,
                0x4D,
                    0x61, 0x70, 0x6E, 0x2E, 0x74, 0x65, 0x6C, 0x63, 0x6F, 0x2E,0x62, 0x61, 0x72,
                0x44,
                    0x75, 0x73, 0x65, 0x72,
                0x46,
                    0x66, 0x6F, 0x6F, 0x62, 0x61, 0x72,
            # server_cfg
            0x0D,
                0x0F, 0x00, 0x00, 0x01,
                0x0F, 0x00,
                0x0F, 0x01, 0x02, 0x03,
                0x12, 0x34,
                0xAB,
            # ctrl_byte
            0xFF,
            # vehicle descriptor
            0x20, 0x36,
                # flags, 0x30 = sim_card_id and auth_key
                0xB1, 0x30,
                0x51, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30,
                0x47, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
                0x85, 0x04, 0x56, 0x00, 0x04, 0x50,
                # sim card id, doesn't match the one used for the test, but that shouldn't be a problem
                0x8A, 0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89,
                # auth key should be set to whatever
                0x08, 0x4D, 0x0D, 0xF4, 0x41, 0x9F, 0x06, 0xB3, 0x85
        ])
        comp_ks = pdu.msg_sign(kt, iccid, date, pdu.msg_read(activation_msg)[0])
        self.assertEquals(8, len(comp_ks))
        self.assertEquals(binascii.b2a_hex(ks), binascii.b2a_hex(comp_ks))

    def test_stdmsg(self):
        from acp245.stdmsg import STANDARD_BY_SECTION_HEX
        for section, m_hex in STANDARD_BY_SECTION_HEX.items():
            try:
                m = pdu.msg_read(hex=m_hex)[0]
            except:
                log.setlevel(9)
                try:
                    m = pdu.msg_read(hex=m_hex)[0]
                except Exception, ex:
                    log.setlevel(0)
                    self.fail('stdmsg "%s" failed: %s' % (section, str(ex)))
            else:
                w_hex = m.as_bytes_hex()

                try:
                    w = pdu.msg_read(hex=w_hex)[0]
                except Exception, e:
                    log.setlevel(9)
                    try:
                        w = pdu.msg_read(hex=w_hex)[0]
                    except Exception, ex:
                        log.setlevel(0)
                        self.fail('stdmsg "%s" failed: %s' % (section, str(ex)))

                if w_hex != m_hex:
                    self.fail(
                        'stdmsg "%s" failed:'
                        'expected %s got %s' % (section, m_hex, w_hex))

                wdict = w.as_dict()
                mdict = m.as_dict()
                if wdict != mdict:
                    mk = mdict.keys()
                    mk.sort()
                    wk = wdict.keys()
                    wk.sort()
                    self.assertEquals(mk, wk)
                    for k in mk:
                        self.assertEqual(mdict[k], wdict[k])

                self.assertEquals(w.as_tree(), m.as_tree())

                if str(m) != str(w):
                    self.fail(
                        'stdmsg "%s" failed:'
                        'expected %s got %s' % (section,
                                                str(m),
                                                str(w)))

    def test_type(self):
        # avoid hitting buf on setup.py and ext_packages when using pxd:
        # http://www.mail-archive.com/cython-dev@codespeak.net/msg05212.html
        # ***
        # The important thing here is NOT the name of the package, but that
        # it matches to the actual package in which they are in Python.
        # If you do import foo.bar.ProvUpd then it's type must be
        # foo.bar.ProvUpd so that things like pickling work.
        # ***
        self.assertEquals(
            "<type 'acp245.pdu_gen.ProvUpd'>",
            str(type(pdu.ProvUpd())))

    def test_is_reply(self):
        self.assert_(pdu.CfgReply().is_reply(pdu.CfgUpd245()))
        self.assert_(pdu.CfgReply245().is_reply(pdu.CfgUpd245()))
        self.assert_(pdu.TrackPos().is_reply(pdu.TrackCmd()))
        self.assert_(pdu.AlarmReply().is_reply(pdu.AlarmNotif()))

    def test_as_tree(self):
        h = pdu.Header(1, 2,
                       test=True,
                       version=4,
                       msg_ctrl=5,
                       msg_prio=6)
        t = h.as_tree()
        self.assertEquals({
            u'type': u'Header',
            u'fields': (
                (u'app_id', 1),
                (u'test', True),
                (u'type', 2),
                (u'version', 4),
                (u'msg_ctrl', 5),
                (u'msg_prio', 6),
            )
        }, t)

        m = pdu.AlarmKA(vehicle_desc=pdu.IEVehicleDesc(iccid='1234'))
        t = m.as_tree()
        self.assertEquals({
            u'type': u'AlarmKA',
            u'fields': (
                (u'header', {
                    u'type': u'Header',
                    u'fields': (
                        (u'app_id', 0xB),
                        (u'test', False),
                        (u'type', 0x4),
                        (u'version', 0),
                        (u'msg_ctrl', 0),
                        (u'msg_prio', 0),
                    )
                }),
                (u'vehicle_desc', {
                    u'type': u'IEVehicleDesc',
                    u'fields': (
                        (u'present', 1),
                        (u'flg1', 128L), (u'flg2', 32),
                        (u'lang', None),
                        (u'model_year', None),
                        (u'vin', None),
                        (u'tcu_serial', None),
                        (u'license_plate', None),
                        (u'vehicle_color', None),
                        (u'vehicle_model', None),
                        (u'imei', None),
                        (u'iccid', u'1234'),
                        (u'auth_key', None)
                    )
                })
            )
        }, t)

    def test_as_dict(self):
        h = pdu.Header(1, 2,
                       test=True,
                       version=4,
                       msg_ctrl=5,
                       msg_prio=6)
        d = h.as_dict()
        self.assertEquals(
            dict(
                app_id=1,
                type=2,
                test=True,
                version=4,
                msg_ctrl=5,
                msg_prio=6
            ), d)

    def test_read_write(self):
        for name in dir(pdu):
            v = getattr(pdu, name)
            if inspect.isclass(v) and pdu.Message in v.__bases__:
                try:
                    m = v()
                    m.as_dict()
                    bytes = m.as_bytes()
                    w_m,sz = pdu.msg_read(bytes)
                except Exception,e:
                    try:
                        log.setlevel(9)
                        m = v()
                        m.as_dict()
                        bytes = m.as_bytes()
                        w_m,sz = pdu.msg_read(bytes)
                    except Exception, ex:
                        self.fail('Failed: %s: %s' % (name, str(ex)))
                    finally:
                        log.setlevel(0)
                else:
                    self.assert_(isinstance(w_m, v))


    def test_msg_thrash(self):
        parser = pdu.Parser()
        seed = random.randint(0, 0xFFFFFFFF)
        sys.stdout.write("random seed: %d" % seed)
        random.seed(seed)
        hdr_data = _to_bin([
            0x01,
            0x01,
            0xB2,
            0x01,
            0x00,
            0x59
        ])
        for i in range(THRASH_CNT):
            l = random.randint(6, 6 + 0xFFFF)
            data = (('%%.%sx'%(l*2)) % random.getrandbits(l*8)).decode('hex')
            try:
                msg, sz = parser.msg_read(hdr_data + data)
            except pdu.MessageError,e:
                pass

    def test_read_write_cfgupd245_no_tcu_desc(self):
        msg = pdu.CfgUpd245(
            appl_flg=3, # change to this application
            tcu_data=pdu.IETCUData(
                items=[
                    pdu.IETCUDataItem(
                        type=0x64,             # server IP 1
                        data=(174,143,242,177) # 174.143.242.177
                    ),
                    pdu.IETCUDataItem(
                        type=0x65,       # server Port 1
                        data=(0x27,0x11) # 10001 (0x2711)
                    ),
                ]
            )
        )
        self.assertEquals(
            '020800160000C000000C006404AE8FF2B10065022711',
            msg.as_bytes_hex()
        )

        msg = pdu.CfgUpd245(
            appl_flg=3, # change to this application
            vehicle_desc=pdu.IEVehicleDesc(
                tcu_serial=pdu.IEAny(data=(0, 1, 14, 0, 7),id=0L),
                iccid='89550534100002116185'
            ),
            tcu_data=pdu.IETCUData(
                items=[
                    pdu.IETCUDataItem(
                        type=0x64,             # server IP 1
                        data=(174,143,242,177) # 174.143.242.177
                    ),
                    pdu.IETCUDataItem(
                        type=0x65,       # server Port 1
                        data=(0x27,0x11) # 10001 (0x2711)
                    ),
                ]
            )
        )
        self.assertEquals(
            '0208002A0000C2001390200500010E00078A89550534100002116185000C006404AE8FF2B10065022711',
            msg.as_bytes_hex()
        )

    def test_read_write_cfgupd245_invalid_TCUDataItem(self):
        msg = pdu.CfgUpd245(
            appl_flg=3, # change to this application
            tcu_data=pdu.IETCUData(
                items=[
                    pdu.IETCUDataItem(
                        type=0x64,             # server IP 1
                        data=(174,143,242,177) # 174.143.242.177
                    ),
                    pdu.IETCUDataItem(
                        type=0x65,       # server Port 1
                        data=0x11        # Invalid Value
                    ),
                ]
            )
        )
        try:
            msg.as_bytes_hex()
            self.fail('Must raise a value error')
        except ValueError:
            pass

    def test_read_write_msg_prov_upd(self):
        hdr_data = _to_bin([
            0x01,
            0x01,
            0xB0,
            0x01,
            0x4E
        ])

        body_data = _to_bin([
            # version element
            0x04, 0x08, 0x83, 0x01, 0x03,
            # target app id
            0x47,
            # appl flg1 - ctrl flg1
            0xAE,
            # ctrl flg 2
            0x00,
            # start time
            0x48, 0xDC, 0x3C, 0x30,
            # end time, 2008, 01, 26, 16:56:23
            0x48, 0xDC, 0x3C, 0x38,
            # tcu descriptor
            0x09, 0x00, 0x1, 0x55, 0x45, 0x30, 0x31, 0x32, 0x33, 0x34,

            # vehicle descriptor
            0x20, 0x2D, 0xB1, 0x20,
            0x51, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30,
            0x47, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
            0x85, 0x04, 0x56, 0x00, 0x04, 0x50,
            0x8A, 0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89,
        ])
        parser = pdu.Parser()
        msg, sz = parser.msg_read(hdr_data + body_data)

        hdr = msg.header
        self.assertEquals(pdu.ACP_APP_ID_PROVISIONING, hdr.app_id);
        self.assertFalse(hdr.test);
        self.assertEquals(pdu.ACP_MSG_TYPE_PROV_UPD, hdr.type);
        self.assertEquals(3, hdr.version);
        self.assertEquals(0, hdr.msg_ctrl);
        self.assertEquals(1, hdr.msg_prio);

        self.assertFalse(hdr.msg_ctrl & pdu.ACP_HDR_MSG_CTRL_RESP_EXP);

        self.assertEquals(2008, msg.start_time.year);
        self.assertEquals(3, msg.start_time.month);
        self.assertEquals(14, msg.start_time.day);
        self.assertEquals(3, msg.start_time.hour);
        self.assertEquals(48, msg.start_time.minute);
        self.assertEquals(48, msg.start_time.second);

        self.assertEquals(2008, msg.end_time.year);
        self.assertEquals(3, msg.end_time.month);
        self.assertEquals(14, msg.end_time.day);
        self.assertEquals(3, msg.end_time.hour);
        self.assertEquals(48, msg.end_time.minute);
        self.assertEquals(56, msg.end_time.second);

        self.assert_(msg.grace_time is None)

        self.assertEquals(0x8, msg.version.car_manufacturer);
        self.assertEquals(0x83, msg.version.tcu_manufacturer);
        self.assertEquals(1, msg.version.major_hard_rel);
        self.assertEquals(3, msg.version.major_soft_rel);

        self.assertEquals(0x55, msg.tcu_desc.device_id);
        self.assertEquals("01234", msg.tcu_desc.version);

        self.assertEquals("0123456789ABCDEF0", msg.vehicle_desc.vin);
        self.assertEquals("1234567", msg.vehicle_desc.tcu_serial);
        self.assertEquals("0456000450", msg.vehicle_desc.imei);
        self.assertEquals("01234567890123456789", msg.vehicle_desc.iccid);

        self.assertEquals(hdr_data, msg.as_bytes()[:len(hdr_data)])
        self.assertEquals(body_data, msg.as_bytes()[len(hdr_data):])

    def test_read_write_msg_prov_reply(self):
        hdr_data = _to_bin([
            0x01,
            0x03,
            0x30,
            0x3D
        ])
        body_data = _to_bin([
            # version element
            0x04, 0x08, 0x83, 0x01, 0x03,
            # target app id
            0x47,
            # appl flg1 - ctrl flg1
            0x02,
            # status flag - tcu resp flag
            0x90,
            # error element
            0x01, 0x03,
            # vehicle descriptor
            0x20, 0x2D, 0xB1, 0x20,
            0x51, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30,
            0x47, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
            0x85, 0x04, 0x56, 0x00, 0x04, 0x50,
            0x8A, 0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89,
        ])

        parser = pdu.Parser()
        msg, sz = parser.msg_read(hdr_data + body_data)

        hdr = msg.header

        self.assertEquals(pdu.ACP_APP_ID_PROVISIONING, hdr.app_id);
        self.assertFalse(hdr.test);
        self.assertEquals(pdu.ACP_MSG_TYPE_PROV_REPLY, hdr.type);
        self.assertEquals(3, hdr.version);
        self.assertEquals(0, hdr.msg_ctrl);
        self.assertEquals(0, hdr.msg_prio);

        self.assertFalse(hdr.msg_ctrl & pdu.ACP_HDR_MSG_CTRL_RESP_EXP);

        self.assertEquals(0x8, msg.version.car_manufacturer);
        self.assertEquals(0x83, msg.version.tcu_manufacturer);
        self.assertEquals(1, msg.version.major_hard_rel);
        self.assertEquals(3, msg.version.major_soft_rel);

        self.assertEquals("0123456789ABCDEF0", msg.vehicle_desc.vin);
        self.assertEquals("1234567", msg.vehicle_desc.tcu_serial);
        self.assertEquals("0456000450", msg.vehicle_desc.imei);
        self.assertEquals("01234567890123456789", msg.vehicle_desc.iccid);

        self.assertEquals(body_data, msg.as_bytes()[len(hdr_data):])
        self.assertEquals(hdr_data, msg.as_bytes()[:len(hdr_data)])

        d = msg.as_dict()

    def test_acp_msg_read_write_cfg_upd_245(self):
        hdr_data = _to_bin([
             0x02,
             0x08,
             0x01,
             0x5C  #92
        ])
        body_data = _to_bin([
        # version element */
        0x04, 0x08, 0x83, 0x01, 0x03,                                                           # 17 */
            # TARGET APPLICATIN ID */
        0x47,
            # appl flg1 - ctrl flg1 */
        0x2E,
            # control flag 2*/
        0x01,
            # reserved */
        0x00,

        # START TIME */
        # Year - Month */
        0x34,    # 2009/03/13 15:00:00  */

        # month - day - hour */
        0xDA,

        # hour - min */
        0xF0,
        # min - sec */
        0x00,
        # END TIME */
        # year - month*/
        0x34,    # 2009/03/14 17:00:00*/

        # month - day - hour*/
        0xDD,

        # hour - min */
        0x10,
        # min - sec */
        0x00,


        # VEHICLE DESCRIPTOR ELEMENT                                                         /* 59 */
        0x20, 0x39,    #lenght 57*/
        0xB1, 0x30,
        #VIN Number*/
        0x50, #IE Identifier = 1 - length = 16 */
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
        #TCU Serial number*/
        0x48, #IE Identifier = 2 - length = 8*/
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        #BCD IMEI Number*/
        0x88, #IE Identifier = 2 - length = 15/2 = 8 bytes*/
        0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67,

        #BCD SIM CARD ID*/
        0x8A,#IE = 2 - length = 19/2 = 10 bytes*/
        0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89,

        #Binary format (Auth Key)*/
        0x08, #IE = 0 - length = 8*/
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,

        # TCU DESCRIPTOR */                                                                    # 12 */
        # IE Identifier - more flag - lenght */
        0x05,
        # reserved */
        0x00,
        # IE Identifier - more flag - lenght */
        0x01,
        # Device ID */
        0x03,
        # IE Identifier - more flag - lenght */
        0x01,
        # version ID */
        0x04,

        # TCU DATA ELEMENT */
        # IE identifier - more flag - length */
        0x05,     # pagina 74, el ultimo*/
        # Data type MSB */
        0x00,
        # Data type LSB */
        0x82,
        # Lenght data type */
        0x02,
        # Configuration data */
        0x02,
        0xD7
        ])

        parser = pdu.Parser()
        msg, sz = parser.msg_read(hdr_data + body_data)

        hdr = msg.header;

        self.assertEquals(pdu.ACP_APP_ID_CONFIGURATION, hdr.app_id);
        self.assertFalse(hdr.test);
        self.assertEquals(pdu.ACP_MSG_TYPE_CFG_UPD_245, hdr.type);
        self.assertEquals(0, hdr.version);
        self.assertEquals(0x1, hdr.msg_ctrl);

        self.assertTrue(hdr.msg_ctrl & pdu.ACP_HDR_MSG_CTRL_RESP_EXP);

        self.assertEquals(0x8, msg.version.car_manufacturer);
        self.assertEquals(0x83, msg.version.tcu_manufacturer);
        self.assertEquals(1, msg.version.major_hard_rel);
        self.assertEquals(3, msg.version.major_soft_rel);

        self.assertEquals("0123456789ABCDEF", msg.vehicle_desc.vin);
        self.assertEquals("01234567", msg.vehicle_desc.tcu_serial);
        self.assertEquals("2345678901234567", msg.vehicle_desc.imei);
        self.assertEquals("01234567890123456789", msg.vehicle_desc.iccid);

        d = msg.as_dict()

        self.assertEquals(body_data, msg.as_bytes()[len(hdr_data):])
        self.assertEquals(hdr_data, msg.as_bytes()[:len(hdr_data)])

        orig = binascii.b2a_hex(hdr_data + body_data)
        written = binascii.b2a_hex(msg.as_bytes())
        self.assertEquals(orig, written)

    def test_acp_msg_cfg_upd_245_py(self):
        msg = pdu.CfgUpd245(
            version=pdu.IEVersion(
                car_manufacturer=0x20,# Car manufacturer is Effa Motors (30)
                tcu_manufacturer=130, # TCU manufacturer is JCI (130)
                # release numbers
                major_hard_rel=0x1,
                major_soft_rel=0x2
            ),
            target_app_id=0x1,
            end_time=pdu.IETimestamp(
                year=2009,
                month=03,
                day=20,
                hour=01,
                minute=03,
                second=04
            ),

            # do not include start time or grace time

            vehicle_desc=pdu.IEVehicleDesc(
                vin="123456789012345",
                # do not include, IMEI, ICCID or Authentication Key
            ),
            tcu_desc=pdu.IETCUDesc(
                device_id=0x20,
                version="0.0.1"
            ),
            tcu_data=pdu.IETCUData(
                items=[
                    pdu.IETCUDataItem(
                        type=0x11,      # transmission interval (0x0011)
                        data=(0x01,0x2C) # 300 seconds (0x12C)
                    ),
                    pdu.IETCUDataItem(
                        type=0x64,              # server IP
                        data=(0xD1, 0x14, 0x55, 0x3D) # 209.20.85.61 (0xD114553D)
                    ),
                    pdu.IETCUDataItem(
                        type=0x65,      # server Port
                        data=(0x2E, 0xE1) # 12001 (0x2EE1)
                    ),
                ]
            )
        ) # end of CfgUpd245
        d = pdu.msg_write(msg)
        msg_readed,sz = pdu.msg_read(d)
        self.assertEquals(msg.as_dict(), msg_readed.as_dict())

    def test_acp_msg_read_write_cfg_reply(self):
        hdr_data = _to_bin([
            0x02,
            0x03,
            0x01,
            0x4A        #74*/
        ])
        body_data = _to_bin([
        # version element */
        0x04, 0x08, 0x83, 0x01, 0x03,                                                   # 5 */
        #RESERVED
        #    0x00,
        # MESSAGE FIELDS                                                                  6 */
        0x00,
        #More flag - Target Application ID*/
        0x00,
        #ApplFlag1 - ControlFlag1 = 2*/
        0x02,
        #StatusFlag1 - TCU Response flag - Reserved*/
        0x00,


        #ERROR ELEMENT*/
        #ID Identifier - More flag - Length*/
        0x01,
        #Error code*/
        0x00,

        # VEHICLE DESCRIPTOR ELEMENT */                                                                /* 59 */
        0x20,0x39,    #lenght 57*/
        0xB1, 0x30,
        #VIN Number*/
        0x50, #IE Identifier = 1 - length = 16 */
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
        #TCU Serial number*/
        0x48, #IE Identifier = 2 - length = 8*/
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        #BCD IMEI Number*/
        0x88, #IE Identifier = 2 - length = 15/2 = 8 bytes*/
        0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67,
        #BCD SIM CARD ID*/
        0x8A,#IE = 2 - length = 19/2 = 10 bytes*/
        0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89,

        #Binary format (Auth Key)*/
        0x08, #IE = 0 - length = 8*/
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
        ])

        parser = pdu.Parser()
        msg, sz = parser.msg_read(hdr_data + body_data)

        hdr = msg.header;

        self.assertEquals(pdu.ACP_APP_ID_CONFIGURATION, hdr.app_id);
        self.assertFalse(hdr.test);
        self.assertEquals(pdu.ACP_MSG_TYPE_CFG_REPLY, hdr.type);
        self.assertEquals(0, hdr.version);
        self.assertEquals(0x1, hdr.msg_ctrl);

        self.assertTrue(hdr.msg_ctrl & pdu.ACP_HDR_MSG_CTRL_RESP_EXP);

        self.assertEquals(0x8, msg.version.car_manufacturer);
        self.assertEquals(0x83, msg.version.tcu_manufacturer);
        self.assertEquals(1, msg.version.major_hard_rel);
        self.assertEquals(3, msg.version.major_soft_rel);

        self.assertEquals("0123456789ABCDEF", msg.vehicle_desc.vin);
        self.assertEquals("01234567", msg.vehicle_desc.tcu_serial);
        self.assertEquals("2345678901234567", msg.vehicle_desc.imei);
        self.assertEquals("01234567890123456789", msg.vehicle_desc.iccid);

        self.assertEquals(body_data, msg.as_bytes()[len(hdr_data):])
        self.assertEquals(hdr_data, msg.as_bytes()[:len(hdr_data)])

        d = msg.as_dict()


    def test_acp_msg_read_write_track_pos(self):
        hdr_data = _to_bin([
            0x0A,
            0x02,
            0x00,
            0x8F    #144*/
        ])
        body_data = _to_bin([
        # version element */
        0x04, 0x08, 0x83, 0x01, 0x03,                                                   # 5 */

        # TIMESTAMP */
        # Year - Month */
        0x34,    # 2009/03/13 15:00:00  */                                             /* 4 */
        # month - day - hour */
        0xDA,
        # hour - min */
        0xF0,
        # min - sec */
        0x00,

        #LOCATION ELEMENT*/                                                            /* 25 */
        #IE Identifier - More flag - Length*/
        0x20, 0x3A,  #LENGTH = 58, more flag = 1; 0x3A = 58*/
        #CURRENT GPSRawData*/

        #IE identifier = 0 - More flag - length*/
        0x16, #LENGTH GPSRawDataElement = 22 */

        #AREA LOCATION CODING*/
        #IE Identifier = 0 - More flag - Length*/
        0x12, #LENGTH Area location coding = 18*/
        #More flag - Area Location Status Flag 1*/
        0xA0,
        #More flag - Area Location Status Flag 2*/
        0x00,
        #Area type - Location type - reserved*/
        0x00,
        #More flag - Time difference*/
        0x00,
        #Longitude byte 1*/
        0xf5,
        #Longitude byte 2*/
        0xe6,
        #Longitude byte 3*/
        0x1d,
        #Longitude byte 4*/
        0x8d,
        #Latitude byte 1*/
        0xfb,
        #Latitude byte 2*/
        0x19,
        #Latitude byte 3*/
        0xed,
        #Latitude byte 4*/
        0xcb,
        #Altitude byte 1*/
        0x03,
        #Altitude byte 2*/
        0x04,
        #  Position uncertany estimate - K/HDOP*/
        0x10,
        #Heading Uncertainty Estimate - Heading*/
        0x31, #15 grados*/
        #Reserved - Distance flag - Time flag*/
        0x00, #units not definded - seconds*/
        #Velocity*/
        0x00,
        #END OF AREA LOCATION CODING*/

        #Number of satelites - reserved*/
        0x20, # 2 satelites*/
        #satelite ID 1 */
        0xFF,
        #satelite ID 2 */
        0xFE,

        #PRIOR GPSRawData*/                                                                    /* 35 */
        #IE identifier = 0 - More flag - length*/
        0x16, # LENGTH = 22 */
        #AREA LOCATION CODING*/
        #IE Identifier = 0 - More flag - Length*/
        0x12, # LENGTH Area Location Coding = 18*/
        #More flag - Area Location Status Flag 1*/
        0xA0,
        #More flag - Area Location Status Flag 2*/
        0x00,
        #Area type - Location type - reserved*/
        0x00,
        #More flag - Time difference*/
        0x00,
        #Longitude byte 1*/
        0x00,
        #Longitude byte 2*/
        0x01,
        #Longitude byte 3*/
        0x02,
        #Longitude byte 4*/
        0x04,
        #Latitude byte 1*/
        0x10,
        #Latitude byte 2*/
        0x11,
        #Latitude byte 3*/
        0x12,
        #Latitude byte 4*/
        0x13,
        #Altitude byte 1*/
        0x0F,
        #Altitude byte 2*/
        0xF0,
        #  Position uncertany estimate - K/HDOP*/
        0x10,
        #Heading Uncertainty Estimate - Heading*/
        0x31, #15 grados*/
        #Reserved - Distance flag - Time flag*/
        0x00, #units not definded - seconds*/
        #Velocity*/
        0x00,

        #END OF AREA LOCATION CODING*/
        #Number of satelites - reserved*/
        0x20, # 2 satelites*/
        #satelite ID 1 */
        0xFF,
        #satelite ID 2 */
        0xFE,

        #Current Dead Reckoning Data*/
        #IE Identifier = 0 - More flag - Length*/
        0x08,  #LENGTH = 2*/
        #Latitude*/
        0x8F,0x8F,0x8F,0x8F,
        #Longitude*/
        0xF0,0xF0,0xF0,0x70,

        #Array of Area Location Delta Coding*/
        #IE Identifier = 0 - More flag - Length*/
        0x02,   #LENGTH = 2*/
        #Delta Longitude 1*/
        0x81,
        #Delta Latitude 1*/
        0x02,
        #END OF LOCATION ELEMENT*/

        # VEHICLE DESCRIPTOR ELEMENT */                                                                /* 59 */
        0x20,0x39,    #lenght 57*/
        0xB1, 0x30,
        #VIN Number*/
        0x50, #IE Identifier = 1 - length = 16 */
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
        #TCU Serial number*/
        0x48, #IE Identifier = 2 - length = 8*/
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        #BCD IMEI Number*/
        0x88, #IE Identifier = 2 - length = 15/2 = 8 bytes*/
        0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67,
        #BCD SIM CARD ID*/
        0x8A,#IE = 2 - length = 19/2 = 10 bytes*/
        0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89,

        #Binary format (Auth Key)*/
        0x08, #IE = 0 - length = 8*/
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,

        #BREAKDOWN STATUS ELEMENT*/                                                                    /* 11 */
        #IE Identifier - More flag - Length*/
        0x06,
        #More flag = 1 - Breakdown Source (First flag)*/
        0x81,
        #More flag = 1 - Breakdown Source (Second  flag)*/
        0x80,
        #More flag - Breakdown Source (Third flag)*/
        0x02,
        #More flag - Breakdown Sensor*/
        0x03,
        #IE Identifier - More flag - Length*/
        0x01,
        #Breakdown Data*/
        0x30,


        #INFORMATION TYPE ELEMENT*/
        #IE Identifier = 0 - More flag - Length */
        0x03,

        #Add Flag - Information Type*/
        0x00,
        #IE Identifier - More flag - Length */
        0x01,
        #Raw Data*/
        0x30
        ])

        parser = pdu.Parser()
        msg, sz = parser.msg_read(hdr_data + body_data)

        hdr = msg.header

        self.assertEquals(pdu.ACP_APP_ID_VEHICLE_TRACKING, hdr.app_id)
        self.assertFalse(hdr.test)
        self.assertEquals(pdu.ACP_MSG_TYPE_TRACK_POS, hdr.type)
        self.assertEquals(0, hdr.version)
        self.assertEquals(0x0, hdr.msg_ctrl)

        self.assertFalse(hdr.msg_ctrl & (pdu.ACP_HDR_MSG_CTRL_16BIT_LEN |
                                 pdu.ACP_HDR_MSG_CTRL_RESP_EXP | pdu.ACP_HDR_MSG_CTRL_DONT_USE_TLV))

        self.assertEquals(0x8, msg.version.car_manufacturer)

        self.assertEquals(0x83, msg.version.tcu_manufacturer)
        self.assertEquals(1, msg.version.major_hard_rel)
        self.assertEquals(3, msg.version.major_soft_rel)

        self.assertEquals(-82186805, msg.location.curr_gps.lat)
        self.assertEquals('-22.829668,-47.074592', '%.6f,%.6f' % msg.location.curr_gps.coords)
        self.assertEquals((0xFF, 0xFE), msg.location.curr_gps.satellites)

        self.assertEquals("0123456789ABCDEF", msg.vehicle_desc.vin)
        self.assertEquals("01234567", msg.vehicle_desc.tcu_serial)
        self.assertEquals("2345678901234567", msg.vehicle_desc.imei)
        self.assertEquals("01234567890123456789", msg.vehicle_desc.iccid)

        self.assertTrue(msg.breakdown_status.present)
        self.assertEquals(3, len(msg.breakdown_status.source))
        self.assertEquals(1, msg.breakdown_status.source[0])
        self.assertEquals(0, msg.breakdown_status.source[1])
        self.assertEquals(2, msg.breakdown_status.source[2])
        self.assertEquals(3, msg.breakdown_status.sensor)

        body = binascii.b2a_hex(body_data)
        hdr = binascii.b2a_hex(hdr_data)
        bytes = binascii.b2a_hex(msg.as_bytes())
        self.assertEquals(body, bytes[len(hdr):])
        self.assertEquals(body_data, msg.as_bytes()[len(hdr_data):])
        self.assertEquals(hdr_data, msg.as_bytes()[:len(hdr_data)])

        d = msg.as_dict()

    def test_acp_msg_read_write_track_pos_info_type(self):
        msg = pdu.TrackPos(
            # reply with the same fields
            version=pdu.IEVersion(
                tcu_manufacturer=0x1,
                car_manufacturer=0x2
            ),
            timestamp=pdu.IETimestamp(
                year=2009,
                month=03,
                day=20,
                hour=12,
                minute=30,
                second=20
            ),
            location=pdu.IELocation(
                curr_gps=pdu.IEGPSRawData(
                    lon=10,
                    lat=20,
                    velocity=30,
                    satellites=[1,2,3]
                )
            ),
            info_type=pdu.IEInfoType(
                type=1,
                data=(0xDE, 0xCD, 0xAB)
            ),
        )
        w_msg,sz = pdu.msg_read(msg.as_bytes())

        # dicts are different, which is OK since some
        # elements were not specified, and therefore
        # None, but they must be sent as empty elements
        # to conform to ACP245 format.
        # self.assertEquals(msg.as_dict(), w_msg.as_dict())


    def test_acp_msg_read_write_cfg_reply_245(self):
        hdr_data = _to_bin([
            0x02,
            0x09,
            0x01,
            0x4E    #78*/
        ])
        body_data = _to_bin([
        # version element */
        0x04, 0x08, 0x83, 0x01, 0x03,                                                   # 5 */

        #MESSAGE FIELDS*/                                                              /* 3 */
        #More flags*/
        0x00,
        #Applflag1 - Control flag = 2*/
        0x02,
        #Status flag1 - TCU Response Flag - Reserved*/
        0x00,

        #TCU DATA ERROR ELEMENT*/                                                      /* 7 */
        #IE Identifier - More Flag - Length*/
        0x06,
        #Data Type MSB*/
        0x00,
        #Data Type LSB*/
        0x11,
        #Length Data Type*/
        0x01,
        #Configuration Data*/
        0x00,
        #Error Element*/
        #IE Identifier = 0 - More flag - Length*/
        0x01,
        #Error code*/
        0x00,


        # VEHICLE DESCRIPTOR ELEMENT */                                                                /* 59 */
        0x20, 0x39,    #lenght 57*/
        0xB1, 0x30,
        #VIN Number*/
        0x50, #IE Identifier = 1 - length = 16 */
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
        #TCU Serial number*/
        0x48, #IE Identifier = 2 - length = 8*/
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        #BCD IMEI Number*/
        0x88, #IE Identifier = 2 - length = 15/2 = 8 bytes*/
        0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67,
        #BCD SIM CARD ID*/
        0x8A,#IE = 2 - length = 19/2 = 10 bytes*/
        0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89,
        #Binary format (Auth Key)*/
        0x08, #IE = 0 - length = 8*/
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
        ])
        parser = pdu.Parser()
        msg, sz = parser.msg_read(hdr_data + body_data)

        hdr = msg.header

        self.assertEquals(pdu.ACP_APP_ID_CONFIGURATION, hdr.app_id)
        self.assertFalse(hdr.test)
        self.assertEquals(pdu.ACP_MSG_TYPE_CFG_REPLY_245, hdr.type)
        self.assertEquals(0, hdr.version)
        self.assertEquals(0x1, hdr.msg_ctrl)

        self.assertTrue(hdr.msg_ctrl & pdu.ACP_HDR_MSG_CTRL_RESP_EXP)
        self.assertEquals(0x8, msg.version.car_manufacturer)
        self.assertEquals(0x83, msg.version.tcu_manufacturer)
        self.assertEquals(1, msg.version.major_hard_rel)
        self.assertEquals(3, msg.version.major_soft_rel)
        self.assertEquals("0123456789ABCDEF", msg.vehicle_desc.vin)
        self.assertEquals("01234567", msg.vehicle_desc.tcu_serial)
        self.assertEquals("2345678901234567", msg.vehicle_desc.imei)
        self.assertEquals("01234567890123456789", msg.vehicle_desc.iccid)

        self.assertEquals(body_data, msg.as_bytes()[len(hdr_data):])
        self.assertEquals(hdr_data, msg.as_bytes()[:len(hdr_data)])

    def test_acp_msg_read_write_cfg_activation(self):
        hdr_data = _to_bin([
            0x02,
            0x0A,
            0x00,
            0x5D
        ])
        body_data = _to_bin([
            # apn_cfg
            0x1A,
                0x4D,
                    0x61, 0x70, 0x6E, 0x2E, 0x74, 0x65, 0x6C, 0x63, 0x6F, 0x2E,0x62, 0x61, 0x72,
                0x44,
                    0x75, 0x73, 0x65, 0x72,
                0x46,
                    0x66, 0x6F, 0x6F, 0x62, 0x61, 0x72,
            # server_cfg
            0x0D,
                0x0F, 0x00, 0x00, 0x01,
                0x0F, 0x00,
                0x0F, 0x01, 0x02, 0x03,
                0x12, 0x34,
                0xAB,
            # ctrl_byte
            0xFF,

            # vehicle descriptor
            0x20, 0x2D, 0xB1, 0x20,
            0x51, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30,
            0x47, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
            0x85, 0x04, 0x56, 0x00, 0x04, 0x50,
            0x8A, 0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89
        ])
        parser = pdu.Parser()
        msg, sz = parser.msg_read(hdr_data + body_data)

        hdr = msg.header

        self.assertEquals(pdu.ACP_APP_ID_CONFIGURATION, hdr.app_id)
        self.assertFalse(hdr.test)
        self.assertEquals(pdu.ACP_MSG_TYPE_CFG_ACT_245, hdr.type)
        self.assertEquals(0, hdr.version)
        self.assertEquals(0x0, hdr.msg_ctrl)

        self.assertEquals("apn.telco.bar", msg.apn_cfg.address);
        self.assertEquals("user", msg.apn_cfg.login);
        self.assertEquals("foobar", msg.apn_cfg.password);

        self.assertEquals('15.0.0.1', msg.server_cfg.server_1);
        self.assertEquals(0x0F00, msg.server_cfg.port_1);
        self.assertEquals('15.1.2.3', msg.server_cfg.server_2);
        self.assertEquals(0x1234, msg.server_cfg.port_2);
        self.assertEquals(0xAB, msg.server_cfg.proto_id);

        self.assertEquals("0123456789ABCDEF0", msg.vehicle_desc.vin);
        self.assertEquals("1234567", msg.vehicle_desc.tcu_serial);
        self.assertEquals("0456000450", msg.vehicle_desc.imei);
        self.assertEquals("01234567890123456789", msg.vehicle_desc.iccid);

        self.assertEquals(body_data, msg.as_bytes()[len(hdr_data):])
        self.assertEquals(hdr_data, msg.as_bytes()[:len(hdr_data)])

    def test_acp_msg_read_write_func_cmd_1_31_1(self):
        hdr_data = _to_bin([
            0x06,
            0x02,
            0x31,
            0x3F    #63
        ])

        body_data = _to_bin([
        # version element */
        0x04, 0x08, 0x83, 0x01, 0x03,                                                   # 5 */
        # control function */
        0x03, 0x0A, 0x00, 0x00,
        # function command */
        0x02, 0x02, 0x00,
        # vehicle descriptor */
        0x20, 0x2D, 0xB1, 0x20,
        0x51, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30,
        0x47, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x85, 0x04, 0x56, 0x00, 0x04, 0x50,
        0x8A, 0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89
        ])

        parser = pdu.Parser()
        msg, sz = parser.msg_read(hdr_data + body_data)

        hdr = msg.header

        self.assertEquals(pdu.ACP_APP_ID_REMOTE_VEHICLE_FUNCTION, hdr.app_id)
        self.assertFalse(hdr.test)
        self.assertEquals(pdu.ACP_MSG_TYPE_FUNC_CMD, hdr.type)
        self.assertEquals(3, hdr.version)
        self.assertEquals(1, hdr.msg_ctrl)
        self.assertEquals(0, hdr.msg_prio)

        self.assertTrue(hdr.msg_ctrl & pdu.ACP_HDR_MSG_CTRL_RESP_EXP)

        self.assertEquals(0x8, msg.version.car_manufacturer)
        self.assertEquals(0x83, msg.version.tcu_manufacturer)
        self.assertEquals(1, msg.version.major_hard_rel)
        self.assertEquals(3, msg.version.major_soft_rel)

        self.assertEquals(pdu.ACP_ENT_ID_IMMOBILIZE, msg.ctrl_func.entity_id)
        self.assertEquals(pdu.ACP_EL_TIME_UNIT_SECOND, msg.ctrl_func.transmit_unit)
        self.assertEquals(0, msg.ctrl_func.transmit_interval)
        self.assertEquals(pdu.ACP_FUNC_CMD_ENABLE, msg.func_cmd.cmd)
        self.assertFalse(msg.func_cmd.raw_data.present)
        self.assert_(msg.func_cmd.raw_data.data is None)

        self.assertEquals("0123456789ABCDEF0", msg.vehicle_desc.vin)
        self.assertEquals("1234567", msg.vehicle_desc.tcu_serial)
        self.assertEquals("0456000450", msg.vehicle_desc.imei)
        self.assertEquals("01234567890123456789", msg.vehicle_desc.iccid)

        self.assertEquals(binascii.b2a_hex(body_data),
                          binascii.b2a_hex(msg.as_bytes()[len(hdr_data):]))
        self.assertEquals(hdr_data, msg.as_bytes()[:len(hdr_data)])

        d = msg.as_dict()

    def test_acp_msg_read_write_func_status_1_31_2(self):
        hdr_data = _to_bin([
            0x06,
            0x03,
            0x31,
            0x42
        ])
        body_data = _to_bin([
        # version element */
        0x04, 0x08, 0x83, 0x01, 0x03,                                                   # 5 */
        # control function */
        0x03, 0x0A, 0x00, 0x00,
        # function command */
        0x03, 0x02, 0x01, 0x20,
        # error element */
        0x01, 0x00,
        # vehicle descriptor */
        0x20, 0x2D, 0xB1, 0x20,
        0x51, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30,
        0x47, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x85, 0x04, 0x56, 0x00, 0x04, 0x50,
        0x8A, 0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89
        ])

        parser = pdu.Parser()
        msg, sz = parser.msg_read(hdr_data + body_data)

        hdr = msg.header

        self.assertEquals(pdu.ACP_APP_ID_REMOTE_VEHICLE_FUNCTION, hdr.app_id)
        self.assertFalse(hdr.test)
        self.assertEquals(pdu.ACP_MSG_TYPE_FUNC_STATUS, hdr.type)
        self.assertEquals(3, hdr.version)
        self.assertEquals(1, hdr.msg_ctrl)
        self.assertEquals(0, hdr.msg_prio)

        self.assertTrue(hdr.msg_ctrl & pdu.ACP_HDR_MSG_CTRL_RESP_EXP)

        self.assertEquals(0x8, msg.version.car_manufacturer)
        self.assertEquals(0x83, msg.version.tcu_manufacturer)
        self.assertEquals(1, msg.version.major_hard_rel)
        self.assertEquals(3, msg.version.major_soft_rel)

        self.assertEquals(pdu.ACP_ENT_ID_IMMOBILIZE, msg.ctrl_func.entity_id)
        self.assertEquals(pdu.ACP_EL_TIME_UNIT_SECOND, msg.ctrl_func.transmit_unit)
        self.assertEquals(0, msg.ctrl_func.transmit_interval)
        self.assertEquals(pdu.ACP_FUNC_STATE_ENABLED, msg.func_status.cmd)
        self.assertEquals((0x20,), msg.func_status.raw_data.data)
        self.assertTrue(msg.func_status.raw_data.present)

        self.assertEquals("0123456789ABCDEF0", msg.vehicle_desc.vin)
        self.assertEquals("1234567", msg.vehicle_desc.tcu_serial)
        self.assertEquals("0456000450", msg.vehicle_desc.imei)
        self.assertEquals("01234567890123456789", msg.vehicle_desc.iccid)

        self.assertEquals(pdu.ACP_ERR_OK, msg.error.code)

        self.assertEquals(body_data, msg.as_bytes()[len(hdr_data):])
        self.assertEquals(hdr_data, msg.as_bytes()[:len(hdr_data)])

    def test_acp_msg_read_write_track_cmd(self):
        hdr_data = _to_bin([
        0x0A,     #                                                      0A*/
        0x01,    #                                                       01*/
        0x00,   #                                                        02*/
        0x4C   #76*/                                                     4C*/
        ])


        body_data = _to_bin([
        # version element */
        0x04, 0x08, 0x83, 0x01, 0x03,                                                   # 5 */
        # control function */
        0x03, 0x01, 0x00, 0x00,
        # function command */
        0x03, 0x02, 0x01, 0x20,

        # VEHICLE DESCRIPTOR ELEMENT */                                                                /* 59 */
        0x20, 0x39,    #lenght 57*/
        0xB1, 0x30,
        #VIN Number*/
        0x50, #IE Identifier = 1 - length = 16 */
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
        #TCU Serial number*/
        0x48, #IE Identifier = 2 - length = 8*/
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        #BCD IMEI Number*/
        0x88, #IE Identifier = 2 - length = 15/2 = 8 bytes*/
        0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67,
        #BCD SIM CARD ID*/
        0x8A,#IE = 2 - length = 19/2 = 10 bytes*/
        0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89,
        #Binary format (Auth Key)*/
        0x08, #IE = 0 - length = 8*/
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
        ])

        parser = pdu.Parser()
        msg, sz = parser.msg_read(hdr_data + body_data)

        hdr = msg.header

        self.assertEquals(pdu.ACP_APP_ID_VEHICLE_TRACKING, hdr.app_id)
        self.assertFalse(hdr.test)
        self.assertEquals(pdu.ACP_MSG_TYPE_TRACK_CMD, hdr.type)
        self.assertEquals(0, hdr.version)
        self.assertEquals(0, hdr.msg_ctrl)

        self.assertFalse(hdr.msg_ctrl & pdu.ACP_HDR_MSG_CTRL_16BIT_LEN)

        self.assertEquals(0x8, msg.version.car_manufacturer)
        self.assertEquals(0x83, msg.version.tcu_manufacturer)
        self.assertEquals(1, msg.version.major_hard_rel)
        self.assertEquals(3, msg.version.major_soft_rel)

        self.assertEquals(pdu.ACP_ENT_ID_VEHICLE_TRACK, msg.ctrl_func.entity_id)
        self.assertTrue(msg.ctrl_func.transmit_present)
        self.assertEquals(pdu.ACP_EL_TIME_UNIT_SECOND, msg.ctrl_func.transmit_unit)
        self.assertEquals(0, msg.ctrl_func.transmit_interval)

        self.assertEquals("0123456789ABCDEF", msg.vehicle_desc.vin)
        self.assertEquals("01234567", msg.vehicle_desc.tcu_serial)
        self.assertEquals("2345678901234567", msg.vehicle_desc.imei)
        self.assertEquals("01234567890123456789", msg.vehicle_desc.iccid)

        self.assertEquals(body_data, msg.as_bytes()[len(hdr_data):])
        self.assertEquals(hdr_data, msg.as_bytes()[:len(hdr_data)])

    def test_acp_msg_read_write_track_pos_1_32_1(self):
        hdr_data = _to_bin([
        0x0A,
        0x02,
        0x31,
        0x63        #99*/
        ])

        body_data = _to_bin([
        # version element */
        0x04, 0x08, 0x83, 0x01, 0x03,                                                   # 5 */
        # timestamp */
        0x88, 0x42, 0x00, 0x00,
        # location */
        #24    16 */
        0x1B,
        # current gps raw data */
        0x17,
        # area location code */
        0x12, 0x80, 0x45, 0x00, 0x00,
        # lon */
        0xFD, 0x39, 0xA7, 0x2C,
        # lat */
        0xFE, 0x99, 0x98, 0x0C,
        # alt */
        0x00, 0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        # satellites */
        0x30,
        0x07,
        0x05,
        0x03,
        # prior gps raw data */
        0x00,
        # current dead reckoning data */
        0x00,
        # area location delta code */
        0x00,

        # vehicle descriptor */                                                        /* 47 */
        0x20, 0x2D, 0xB1, 0x20,
        0x51, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30,
        0x47, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x85, 0x04, 0x56, 0x00, 0x04, 0x50,
        0x8A, 0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89,

        # BREAKDOWN STATUS ELEMENT */                                                          /* 11 */
        # IE Identifier - More flag - Length */
        0x06,
        # More flag = 1 - Breakdown source (first flag)*/
        0x80,
        # More flag = 1 - Breakdown source (second flag)*/
        0x80,
        # More flag - Breakdown source (third flag)*/
        0x00,
        # More flag - Breakdon sensor*/
        0x00,
        # IE identifier - More flag - Length*/
        0x01,
        # Breakdown data*/
        0x00,

        # INFORMATION TYPE ELEMENT */
        # IE Identifier - More flag - Length*/
        0x03,
        # Add flag - Information type*/
        0x00,
        # IE Identifier - More flag - Length*/
        0x01,
        # Raw data*/
        0x00

        ])

        parser = pdu.Parser()
        msg, sz = parser.msg_read(hdr_data + body_data)

        hdr = msg.header


        self.assertEquals(pdu.ACP_APP_ID_VEHICLE_TRACKING, hdr.app_id)
        self.assertFalse(hdr.test)
        self.assertEquals(pdu.ACP_MSG_TYPE_TRACK_POS, hdr.type)
        self.assertEquals(3, hdr.version)
        self.assertEquals(1, hdr.msg_ctrl)
        self.assertEquals(0, hdr.msg_prio)

        self.assertTrue(hdr.msg_ctrl & pdu.ACP_HDR_MSG_CTRL_RESP_EXP)


        self.assertEquals(0x8, msg.version.car_manufacturer)
        self.assertEquals(0x83, msg.version.tcu_manufacturer)
        self.assertEquals(1, msg.version.major_hard_rel)
        self.assertEquals(3, msg.version.major_soft_rel)

        self.assertEquals(pdu.ACP_MORE_FLG, msg.location.curr_gps.flg1)
        self.assertEquals(0x45, msg.location.curr_gps.flg2)
        self.assertEquals(pdu.ACP_LOCATION_SOUTH_WEST,
                          msg.location.curr_gps.flg2 & pdu.ACP_LOCATION_FLG2_HEAD_MASK)
        self.assertEquals(0, msg.location.curr_gps.area_type)
        self.assertEquals(0, msg.location.curr_gps.location_type)
        self.assertEquals(-46553300, msg.location.curr_gps.lon)
        self.assertEquals(-23488500, msg.location.curr_gps.lat)
        self.assertEquals(0x0, msg.location.curr_gps.alt)
        self.assertEquals(0x0, msg.location.curr_gps.pos_uncert)
        self.assertEquals(0x0, msg.location.curr_gps.head_uncert)
        self.assertEquals(0x0, msg.location.curr_gps.heading)
        self.assertEquals(pdu.ACP_LOCATION_DIST_UNIT_ND, msg.location.curr_gps.dist_unit)
        self.assertEquals(pdu.ACP_LOCATION_TIME_UNIT_SECONDS, msg.location.curr_gps.time_unit)

        # satellite data */
        if pdu.ACP_EL_GPS_RAW_DATA_SAT_MAX > 0:
            self.assertEquals(0x07, msg.location.curr_gps.satellites[0])

        if pdu.ACP_EL_GPS_RAW_DATA_SAT_MAX > 1:
            self.assertEquals(0x05, msg.location.curr_gps.satellites[1])

        if pdu.ACP_EL_GPS_RAW_DATA_SAT_MAX > 2:
            self.assertEquals(0x03, msg.location.curr_gps.satellites[2])

        self.assertEquals(0, msg.location.prev_gps.area_type)
        self.assertEquals(0, msg.location.prev_gps.location_type)
        self.assertEquals(0, msg.location.prev_gps.lon)
        self.assertEquals(0, msg.location.prev_gps.lat)
        self.assertEquals(0x0, msg.location.prev_gps.alt)
        self.assertEquals(0x0, msg.location.prev_gps.pos_uncert)
        self.assertEquals(0x0, msg.location.prev_gps.head_uncert)
        self.assertEquals(0x0, msg.location.prev_gps.heading)
        self.assertEquals(0, msg.location.prev_gps.dist_unit)
        self.assertEquals(0, msg.location.prev_gps.time_unit)

        self.assertEquals("0123456789ABCDEF0", msg.vehicle_desc.vin)
        self.assertEquals("1234567", msg.vehicle_desc.tcu_serial)
        self.assertEquals("0456000450", msg.vehicle_desc.imei)
        self.assertEquals("01234567890123456789", msg.vehicle_desc.iccid)

        self.assertEquals(body_data, msg.as_bytes()[len(hdr_data):])
        self.assertEquals(hdr_data, msg.as_bytes()[:len(hdr_data)])

    def test_acp_msg_read_write_track_reply(self):
        hdr_data = _to_bin([
        0x0A,
        0x03,
        0x01,
        0x0D    #13*/
        ])


        body_data = _to_bin([
        # version element */
        0x04, 0x08, 0x83, 0x01, 0x03,                                                   # 5 */
        #MESSAGE FIELDS*/
        #Confirmation - Transmit units*/
        0x40,
        #E-call control flag2*/
        0x00,
        #ERROR ELEMENT*/
        #IE Identifier - More flag - Length*/
        0x01,
        #Error code*/
        0x00

        ])

        parser = pdu.Parser()
        msg, sz = parser.msg_read(hdr_data + body_data)

        hdr = msg.header

        self.assertEquals(pdu.ACP_APP_ID_VEHICLE_TRACKING, hdr.app_id)
        self.assertFalse(hdr.test)
        self.assertEquals(pdu.ACP_MSG_TYPE_TRACK_REPLY, hdr.type)
        self.assertEquals(0, hdr.version)
        self.assertEquals(1, hdr.msg_ctrl)

        self.assertTrue(hdr.msg_ctrl & pdu.ACP_HDR_MSG_CTRL_RESP_EXP)

        self.assertEquals(0x8, msg.version.car_manufacturer)
        self.assertEquals(0x83, msg.version.tcu_manufacturer)
        self.assertEquals(1, msg.version.major_hard_rel)
        self.assertEquals(3, msg.version.major_soft_rel)

        self.assertEquals(body_data, msg.as_bytes()[len(hdr_data):])
        self.assertEquals(hdr_data, msg.as_bytes()[:len(hdr_data)])

    def test_acp_msg_read_write_track_reply(self):
        hdr_data = _to_bin([
        0x0B,
        0x01,
        0x01,
        0x8F    #143*/
        ])

        body_data = _to_bin([
        # version element */
        0x04, 0x08, 0x83, 0x01, 0x03,                                                   # 5 */
        # TIMESTAMP */
        # Year - Month */
        0x34,    # 2009/03/13 15:00:00  */                                             /* 4 */
        # month - day - hour */
        0xDA,
        # hour - min */
        0xF0,
        # min - sec */
        0x00,

        #LOCATION ELEMENT*/                                                            /* 25 */
        #IE Identifier - More flag - Length*/
        0x20, 0x3A,  #LENGTH = 58, more flag = 1; 0x3A = 58*/
        #CURRENT GPSRawData*/

        #IE identifier = 0 - More flag - length*/
        0x16, #LENGTH GPSRawDataElement = 22 */

        #AREA LOCATION CODING*/
        #IE Identifier = 0 - More flag - Length*/
        0x12, #LENGTH Area location coding = 18*/
        #More flag - Area Location Status Flag 1*/
        0xA0,
        #More flag - Area Location Status Flag 2*/
        0x00,
        #Area type - Location type - reserved*/
        0x00,
        #More flag - Time difference*/
        0x00,
        #Longitude byte 1*/
        0x00,
        #Longitude byte 2*/
        0x01,
        #Longitude byte 3*/
        0x02,
        #Longitude byte 4*/
        0x04,
        #Latitude byte 1*/
        0x10,
        #Latitude byte 2*/
        0x11,
        #Latitude byte 3*/
        0x12,
        #Latitude byte 4*/
        0x13,
        #Altitude byte 1*/
        0x03,
        #Altitude byte 2*/
        0x04,
        #  Position uncertany estimate - K/HDOP*/
        0x10,
        #Heading Uncertainty Estimate - Heading*/
        0x31, #15 grados*/
        #Reserved - Distance flag - Time flag*/
        0x00, #units not definded - seconds*/
        #Velocity*/
        0x00,
        #END OF AREA LOCATION CODING*/

        #Number of satelites - reserved*/
        0x20, # 2 satelites*/
        #satelite ID 1 */
        0xFF,
        #satelite ID 2 */
        0xFE,

        #PRIOR GPSRawData*/                                                                    /* 35 */
        #IE identifier = 0 - More flag - length*/
        0x16, # LENGTH = 22 */
        #AREA LOCATION CODING*/
        #IE Identifier = 0 - More flag - Length*/
        0x12, # LENGTH Area Location Coding = 18*/
        #More flag - Area Location Status Flag 1*/
        0xA0,
        #More flag - Area Location Status Flag 2*/
        0x00,
        #Area type - Location type - reserved*/
        0x00,
        #More flag - Time difference*/
        0x00,
        #Longitude byte 1*/
        0x00,
        #Longitude byte 2*/
        0x01,
        #Longitude byte 3*/
        0x02,
        #Longitude byte 4*/
        0x04,
        #Latitude byte 1*/
        0x10,
        #Latitude byte 2*/
        0x11,
        #Latitude byte 3*/
        0x12,
        #Latitude byte 4*/
        0x13,
        #Altitude byte 1*/
        0x0F,
        #Altitude byte 2*/
        0xF0,
        #  Position uncertany estimate - K/HDOP*/
        0x10,
        #Heading Uncertainty Estimate - Heading*/
        0x31, #15 grados*/
        #Reserved - Distance flag - Time flag*/
        0x00, #units not definded - seconds*/
        #Velocity*/
        0x00,

        #END OF AREA LOCATION CODING*/
        #Number of satelites - reserved*/
        0x20, # 2 satelites*/
        #satelite ID 1 */
        0xFF,
        #satelite ID 2 */
        0xFE,

        #Current Dead Reckoning Data*/
        #IE Identifier = 0 - More flag - Length*/
        0x08,  #LENGTH = 2*/
        #Latitude*/
        0x8F,0x8F,0x8F,0x8F,
        #Longitude*/
        0xF0,0xF0,0xF0,0x70,

        #Array of Area Location Delta Coding*/
        #IE Identifier = 0 - More flag - Length*/
        0x02,   #LENGTH = 2*/
        #Delta Longitude 1*/
        0x81,
        #Delta Latitude 1*/
        0x02,
        #END OF LOCATION ELEMENT*/

        # VEHICLE DESCRIPTOR ELEMENT */                                                                /* 59 */
        0x20,0x39,    #lenght 57*/
        0xB1, 0x30,
        #VIN Number*/
        0x50, #IE Identifier = 1 - length = 16 */
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
        #TCU Serial number*/
        0x48, #IE Identifier = 2 - length = 8*/
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        #BCD IMEI Number*/
        0x88, #IE Identifier = 2 - length = 15/2 = 8 bytes*/
        0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67,
        #BCD SIM CARD ID*/
        0x8A,#IE = 2 - length = 19/2 = 10 bytes*/
        0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89,

        #Binary format (Auth Key)*/
        0x08, #IE = 0 - length = 8*/
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,

        #BREAKDOWN STATUS ELEMENT*/                                                                    /* 11 */
        #IE Identifier - More flag - Length*/
        0x06,
        #More flag = 1 - Breakdown Source (First flag)*/
        0x81,
        #More flag = 1 - Breakdown Source (Second  flag)*/
        0x81,
        #More flag - Breakdown Source (Third flag)*/
        0x01,
        #More flag - Breakdown Sensor*/
        0x01,
        #IE Identifier - More flag - Length*/
        0x01,
        #Breakdown Data*/
        0x30,


        #INFORMATION TYPE ELEMENT*/
        #IE Identifier = 0 - More flag - Length */
        0x03,

        #Add Flag - Information Type*/
        0x00,
        #IE Identifier - More flag - Length */
        0x01,
        #Raw Data*/
        0x30
        ])

        parser = pdu.Parser()
        msg, sz = parser.msg_read(hdr_data + body_data)

        hdr = msg.header

        self.assertEquals(pdu.ACP_APP_ID_ALARM , hdr.app_id)
        self.assertFalse(hdr.test)
        self.assertEquals(pdu.ACP_MSG_TYPE_ALARM_NOTIF, hdr.type)
        self.assertEquals(0, hdr.version)
        self.assertEquals(1, hdr.msg_ctrl)

        self.assertTrue(hdr.msg_ctrl & pdu.ACP_HDR_MSG_CTRL_RESP_EXP)

        self.assertEquals(0x8, msg.version.car_manufacturer)
        self.assertEquals(0x83, msg.version.tcu_manufacturer)
        self.assertEquals(1, msg.version.major_hard_rel)
        self.assertEquals(3, msg.version.major_soft_rel)

        self.assertEquals(hdr_data, msg.as_bytes()[:len(hdr_data)])
        self.assertEquals(body_data, msg.as_bytes()[len(hdr_data):])
        self.assertEquals(hdr_data, msg.as_bytes()[:len(hdr_data)])

    def test_acp_msg_read_write_alarm_reply(self):
        hdr_data = _to_bin([
        0x0B,
        0x02,
        0x01,
        0x0D    #13*/
        ])

        body_data = _to_bin([
        # version element */
        0x04, 0x08, 0x83, 0x01, 0x03,                                                   # 5 */

        #MESSAGE FIELDS*/
        #Confirmation - Transmit units*/
        0x40,
        #E-call control flag2*/
        0x00,
        #ERROR ELEMENT*/
        #IE Identifier - More flag - Length*/
        0x01,
        #Error code*/
        0x00
        ])

        parser = pdu.Parser()
        msg, sz = parser.msg_read(hdr_data + body_data)

        hdr = msg.header

        self.assertEquals(pdu.ACP_APP_ID_ALARM, hdr.app_id)
        self.assertFalse(hdr.test)
        self.assertEquals(pdu.ACP_MSG_TYPE_ALARM_REPLY, hdr.type)
        self.assertEquals(0, hdr.version)
        self.assertEquals(1, hdr.msg_ctrl)

        self.assertTrue(hdr.msg_ctrl & pdu.ACP_HDR_MSG_CTRL_RESP_EXP)

        self.assertEquals(0x8, msg.version.car_manufacturer)
        self.assertEquals(0x83, msg.version.tcu_manufacturer)
        self.assertEquals(1, msg.version.major_hard_rel)
        self.assertEquals(3, msg.version.major_soft_rel)

        self.assertEquals(body_data, msg.as_bytes()[len(hdr_data):])
        self.assertEquals(hdr_data, msg.as_bytes()[:len(hdr_data)])

    def test_acp_msg_read_write_alarm_ka(self):
        hdr_data = _to_bin([
            0x0B,
            0x04,
            0x01,
            0x33    # 51
        ])

        body_data = _to_bin([
            # vehicle descriptor, len=47
            0x20, 0x2D, 0xB1, 0x20,
            0x51, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30,
            0x47, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
            0x85, 0x04, 0x56, 0x00, 0x04, 0x50,
            0x8A, 0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89
        ])

        parser = pdu.Parser()
        msg, sz = parser.msg_read(hdr_data + body_data)

        hdr = msg.header

        self.assertEquals(pdu.ACP_APP_ID_ALARM, hdr.app_id)
        self.assertFalse(hdr.test)
        self.assertEquals(pdu.ACP_MSG_TYPE_ALARM_KA, hdr.type)
        self.assertEquals(0, hdr.version)
        self.assertEquals(1, hdr.msg_ctrl)

        self.assertTrue(hdr.msg_ctrl & pdu.ACP_HDR_MSG_CTRL_RESP_EXP)

        self.assertEquals("0123456789ABCDEF0", msg.vehicle_desc.vin);
        self.assertEquals("1234567", msg.vehicle_desc.tcu_serial);
        self.assertEquals("0456000450", msg.vehicle_desc.imei);
        self.assertEquals("01234567890123456789", msg.vehicle_desc.iccid);

        self.assertEquals(body_data, msg.as_bytes()[len(hdr_data):])
        self.assertEquals(hdr_data, msg.as_bytes()[:len(hdr_data)])

    def test_acp_msg_read_write_alarm_ka_no_vehicle_desc(self):
        hdr_data = _to_bin([
            0x0B,
            0x04,
            0x01,
            0x4
        ])
        parser = pdu.Parser()
        msg, sz = parser.msg_read(hdr_data)

        hdr = msg.header

        self.assertEquals(pdu.ACP_APP_ID_ALARM, hdr.app_id)
        self.assertFalse(hdr.test)
        self.assertEquals(pdu.ACP_MSG_TYPE_ALARM_KA, hdr.type)
        self.assertEquals(0, hdr.version)
        self.assertEquals(1, hdr.msg_ctrl)

        self.assertTrue(hdr.msg_ctrl & pdu.ACP_HDR_MSG_CTRL_RESP_EXP)

        self.assertTrue(msg.vehicle_desc is None);

        self.assertEquals(hdr_data, msg.as_bytes())

    def test_acp_msg_read_write_alarm_ka_reply(self):
        hdr_data = _to_bin([
            0x0B,
            0x05,
            0x00,
            0x33    # 51
        ])

        body_data = _to_bin([
            # vehicle descriptor, len=47
            0x20, 0x2D, 0xB1, 0x20,
            0x51, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30,
            0x47, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
            0x85, 0x04, 0x56, 0x00, 0x04, 0x50,
            0x8A, 0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89
        ])

        parser = pdu.Parser()
        msg, sz = parser.msg_read(hdr_data + body_data)

        hdr = msg.header

        self.assertEquals(pdu.ACP_APP_ID_ALARM, hdr.app_id)
        self.assertFalse(hdr.test)
        self.assertEquals(pdu.ACP_MSG_TYPE_ALARM_KA_REPLY, hdr.type)
        self.assertEquals(0, hdr.version)
        self.assertEquals(0, hdr.msg_ctrl)

        self.assertFalse(hdr.msg_ctrl & pdu.ACP_HDR_MSG_CTRL_RESP_EXP)

        self.assertEquals("0123456789ABCDEF0", msg.vehicle_desc.vin);
        self.assertEquals("1234567", msg.vehicle_desc.tcu_serial);
        self.assertEquals("0456000450", msg.vehicle_desc.imei);
        self.assertEquals("01234567890123456789", msg.vehicle_desc.iccid);

        self.assertEquals(body_data, msg.as_bytes()[len(hdr_data):])
        self.assertEquals(hdr_data, msg.as_bytes()[:len(hdr_data)])

    def test_acp_msg_read_write_alarm_ka_reply_no_vehicle_desc(self):
        hdr_data = _to_bin([
            0x0B,
            0x05,
            0x00,
            0x4
        ])
        parser = pdu.Parser()
        msg, sz = parser.msg_read(hdr_data)

        hdr = msg.header

        self.assertEquals(pdu.ACP_APP_ID_ALARM, hdr.app_id)
        self.assertFalse(hdr.test)
        self.assertEquals(pdu.ACP_MSG_TYPE_ALARM_KA_REPLY, hdr.type)
        self.assertEquals(0, hdr.version)
        self.assertEquals(0, hdr.msg_ctrl)

        self.assertFalse(hdr.msg_ctrl & pdu.ACP_HDR_MSG_CTRL_RESP_EXP)

        self.assertTrue(msg.vehicle_desc is None);

        self.assertEquals(hdr_data, msg.as_bytes())

    def test_vehicle_descriptor_flgs(self):
        msg,sz = pdu.msg_read(hex='0B04000A051043616263')
        self.assertEquals(0x10, msg.vehicle_desc.flg1)
        self.assert_(msg.vehicle_desc.flg2 is None)
        msg,sz = pdu.msg_read(hex='0B04000B06900043616263')
        self.assertEquals(0x90, msg.vehicle_desc.flg1)
        self.assertEquals(0, msg.vehicle_desc.flg2)
        self.assertEquals('0B04000B06900043616263', msg.as_bytes_hex())
        msg.vehicle_desc.flg2 = None
        self.assertEquals('0B04000A051043616263', msg.as_bytes_hex())
        msg.vehicle_desc.model_year = 1
        self.assertEquals('0B04000C0790400143616263', msg.as_bytes_hex())
        msg.vehicle_desc.model_year = 0
        self.assertEquals('0B04000C0790400043616263', msg.as_bytes_hex())
        msg.vehicle_desc.model_year = None
        self.assertEquals('0B04000B06900043616263', msg.as_bytes_hex())

    def test_vehicle_descriptor_serial_formats(self):
        for v, expected in (
                ('abc', '0B04000A051043616263'),
                (192, '0B0400090410820192'),
                (pdu.IEAny(id=pdu.ACP_IE_BINARY,
                           data='\x01\x02\xFF'), '0B04000A0510030102FF')):
            bytes = pdu.AlarmKA(
                vehicle_desc=pdu.IEVehicleDesc(
                    tcu_serial=v)).as_bytes_hex()
            self.assertEquals(expected, bytes)
            m = pdu.msg_read(hex = bytes)[0]
            if isinstance(v, pdu.IEAny):
                self.assertEquals(m.vehicle_desc.tcu_serial.id, v.id)
                self.assertEquals(m.vehicle_desc.tcu_serial.data,
                                  tuple([ord(x) for x in v.data]))
                self.assertEquals(str(m.vehicle_desc.tcu_serial), v.data)
            else:
                self.assertEquals(m.vehicle_desc.tcu_serial, v)

    def test_pos_satellites(self):
        r = pdu.TrackPos(
            location=pdu.IELocation(
                curr_gps=pdu.IEGPSRawData(
                    lon=10,
                    lat=20,
                    velocity=30,
                    satellites=[1,2,3]
                )
            ),
        )
        r,s = pdu.msg_read(r.as_bytes())
        self.assertEquals((1,2,3), r.location.curr_gps.satellites)
        r.location.curr_gps.satellites = (4,5,6,7,8)
        self.assertEquals(5, r.location.curr_gps.satellites_avail)
        r,s = pdu.msg_read(r.as_bytes())
        self.assertEquals((4,5,6,7,8), r.location.curr_gps.satellites)


    def test_acp_msg_read_write_extended_version_field(self):
        data = _to_bin([
            0x0B,
            0x04,
            0xF1, # version_flag = 1, version == 7
            0x50, # extended version = 20
            0x05
        ])
        parser = pdu.Parser()
        msg, sz = parser.msg_read(data)

        hdr = msg.header
        self.assertEquals(pdu.ACP_APP_ID_ALARM, hdr.app_id);
        self.assertFalse(hdr.test);
        self.assertEquals(pdu.ACP_MSG_TYPE_ALARM_KA, hdr.type);
        self.assertEquals(20, hdr.version);
        self.assertEquals(1, hdr.msg_ctrl);

        self.assertTrue(hdr.msg_ctrl & pdu.ACP_HDR_MSG_CTRL_RESP_EXP);

        self.assertEquals(data, msg.as_bytes())

    def test_ie_timestamp(self):
        ie = pdu.IETimestamp(time=1253214431)
        self.assertEquals(1253214431, ie.get_time())
        self.assertEquals(2009, ie.year)
        self.assertEquals(9, ie.month)
        self.assertEquals(17, ie.day)
        self.assertEquals(19, ie.hour)
        self.assertEquals(7, ie.minute)
        self.assertEquals(11, ie.second)

    def test_ie_gpsrawdata(self):
        ie = pdu.IEGPSRawData(coords=(-23.55, -46.63))
        self.assertEquals((-23.55, -46.63), ie.coords)
        self.assertEquals(-84780000, ie.lat)
        self.assertEquals(-167868000, ie.lon)

        ie = pdu.IEGPSRawData(lat=-84780000,lon=-167868000)
        self.assertEquals((-23.55, -46.63), ie.coords)
        self.assertEquals(-84780000, ie.lat)
        self.assertEquals(-167868000, ie.lon)

        ie = pdu.IEGPSRawData(area_type=pdu.ACP_LOCATION_POINT_1_MILLIARC,
                              lat=-84780000,lon=-167868000)
        self.assertEquals((-23.55, -46.63), ie.coords)
        self.assertEquals(-84780000, ie.lat)
        self.assertEquals(-167868000, ie.lon)

        ie = pdu.IEGPSRawData(area_type=pdu.ACP_LOCATION_POINT_100_MILLIARC,
                              lat=-847800,lon=-1678680)
        self.assertEquals((-23.55, -46.63), ie.coords)
        self.assertEquals(-847800, ie.lat)
        self.assertEquals(-1678680, ie.lon)

        ie = pdu.IEGPSRawData(area_type=pdu.ACP_LOCATION_POINT_100_MILLIARC,
                              coords=(-23.55, -46.63))
        self.assertEquals((-23.55, -46.63), ie.coords)
        self.assertEquals(-847800, ie.lat)
        self.assertEquals(-1678680, ie.lon)

    def test_ie_gpsrawdata_satellites(self):
        ie = pdu.IEGPSRawData(satellites=None)
        self.assertEquals(tuple(), ie.satellites)
        ie.satellites = (1,2)
        self.assertEquals((1,2), ie.satellites)
        ie = pdu.IEGPSRawData(satellites=(1,2))
        self.assertEquals((1,2), ie.satellites)
        ie = pdu.IEGPSRawData(satellites=[])
        self.assertEquals(tuple(), ie.satellites)
        ie = pdu.IEGPSRawData(satellites=[1])
        self.assertEquals((1,), ie.satellites)
        ie.satellites = (1,2)
        self.assertEquals((1,2), ie.satellites)

    def test_is_valid_tcu_msg(self):
        self.assert_(pdu.AlarmKA().is_valid_tcu_msg())
        self.assert_(pdu.AlarmKA(vehicle_desc=pdu.IEVehicleDesc()).is_valid_tcu_msg())
        self.assert_(pdu.AlarmKA(vehicle_desc=pdu.IEVehicleDesc(iccid='1')).is_valid_tcu_msg())

        self.assertFalse(pdu.ProvReply().is_valid_tcu_msg())
        self.assertFalse(pdu.ProvReply(vehicle_desc=pdu.IEVehicleDesc()).is_valid_tcu_msg())
        self.assertTrue(pdu.ProvReply(
            vehicle_desc=pdu.IEVehicleDesc(iccid='1')).is_valid_tcu_msg())

        self.assertFalse(pdu.ProvUpd().is_valid_tcu_msg())
        self.assertFalse(pdu.ProvUpd(vehicle_desc=pdu.IEVehicleDesc()).is_valid_tcu_msg())
        self.assertFalse(pdu.ProvUpd(vehicle_desc=pdu.IEVehicleDesc(iccid='1')).is_valid_tcu_msg())

    def test_is_valid_tcu_first_msg(self):
        self.assertFalse(pdu.AlarmKA().is_valid_tcu_first_msg())
        self.assertFalse(pdu.AlarmKA(
            vehicle_desc=pdu.IEVehicleDesc()).is_valid_tcu_first_msg())
        self.assertFalse(pdu.AlarmKA(
            vehicle_desc=pdu.IEVehicleDesc(iccid='1')).is_valid_tcu_first_msg())

        self.assertFalse(pdu.ProvReply().is_valid_tcu_first_msg())
        self.assertFalse(pdu.ProvReply(
            vehicle_desc=pdu.IEVehicleDesc()).is_valid_tcu_first_msg())
        self.assertFalse(pdu.ProvReply(
            vehicle_desc=pdu.IEVehicleDesc(),
            version=pdu.IEVersion()).is_valid_tcu_first_msg())
        self.assertFalse(pdu.ProvReply(
            vehicle_desc=pdu.IEVehicleDesc(iccid='1')).is_valid_tcu_first_msg())
        self.assertTrue(pdu.ProvReply(
            vehicle_desc=pdu.IEVehicleDesc(iccid='1'),
            version=pdu.IEVersion()).is_valid_tcu_first_msg())
        self.assertFalse(pdu.ProvReply(
            vehicle_desc=pdu.IEVehicleDesc(iccid='1'),
            version=pdu.IEVersion(present=0)).is_valid_tcu_first_msg())

        self.assertFalse(pdu.ProvUpd().is_valid_tcu_first_msg())
        self.assertFalse(pdu.ProvUpd(
            vehicle_desc=pdu.IEVehicleDesc()).is_valid_tcu_first_msg())
        self.assertFalse(pdu.ProvUpd(
            vehicle_desc=pdu.IEVehicleDesc(iccid='1')).is_valid_tcu_first_msg())
        self.assertFalse(pdu.ProvUpd(
            vehicle_desc=pdu.IEVehicleDesc(iccid='1'),
            version=pdu.IEVersion()).is_valid_tcu_first_msg())

    def test_is_valid_so_msg(self):
        self.assert_(pdu.AlarmKAReply().is_valid_so_msg())
        self.assert_(pdu.AlarmKAReply(
            vehicle_desc=pdu.IEVehicleDesc()).is_valid_so_msg())
        self.assert_(pdu.AlarmKAReply(
            vehicle_desc=pdu.IEVehicleDesc(iccid='1')).is_valid_so_msg())

        self.assertFalse(pdu.ProvUpd().is_valid_so_msg())
        self.assertFalse(pdu.ProvUpd(
            vehicle_desc=pdu.IEVehicleDesc()).is_valid_so_msg())
        self.assertTrue(pdu.ProvUpd(
            vehicle_desc=pdu.IEVehicleDesc(iccid='1')).is_valid_so_msg())

        self.assertFalse(pdu.ProvReply().is_valid_tcu_msg())
        self.assertFalse(pdu.ProvReply(
            vehicle_desc=pdu.IEVehicleDesc()).is_valid_so_msg())
        self.assertFalse(pdu.ProvReply(
            vehicle_desc=pdu.IEVehicleDesc(iccid='1')).is_valid_so_msg())
