#/*=============================================================================
#        Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY
#
#        This software is furnished under a license and may be used and copied
#        only in accordance with the terms of such license and with the
#        inclusion of the above copyright notice. This software or any other
#        copies thereof may not be provided or otherwise made available to any
#        other person. No title to and ownership of the software is hereby
#        transferred.
#==============================================================================*/

import calendar

import twisted.trial.unittest as unittest

from acp245 import pdu
from acpyd.args import \
        insert_args_in_msg, insert_msg_in_args, \
        RequiredError, FormatError, \
        Int8, Time, String, BinHex, List

import acpyd.test.compat

class ArgsTest(unittest.TestCase):
    def test_Int8(self):
        descriptors = (
            ('car_manufacturer_id', Int8('version.car_manufacturer')),
            ('tcu_manufacturer_id', Int8('version.tcu_manufacturer')),
            ('major_hardware_release', Int8('version.major_hard_rel')),
            ('major_software_release', Int8('version.major_soft_rel')),
        )

        msg = insert_args_in_msg(descriptors,
                                {},
                                pdu.ProvUpd())
        self.assert_(msg.version is None)
        self.assert_(not insert_msg_in_args(descriptors, msg))

        msg = insert_args_in_msg(descriptors,
                                {'car_manufacturer_id': 9},
                                pdu.ProvUpd())
        self.assertEquals(9, msg.version.car_manufacturer)
        self.assertEquals(
            '9', insert_msg_in_args(descriptors, msg)['car_manufacturer_id'])

        msg = insert_args_in_msg(descriptors,
                                {'car_manufacturer_id': '10',
                                 'major_hardware_release': 11},
                                pdu.ProvUpd())
        self.assertEquals(10, msg.version.car_manufacturer)
        self.assertEquals(11, msg.version.major_hard_rel)
        self.assertEquals(
            '10', insert_msg_in_args(descriptors, msg)['car_manufacturer_id'])
        self.assertEquals(
            '11', insert_msg_in_args(descriptors, msg)['major_hardware_release'])

        try:
            insert_args_in_msg(descriptors,
                              {'car_manufacturer_id': 'foobar'},
                              pdu.ProvUpd())
        except FormatError:
            pass
        else:
            self.fail()

    def test_Int8_with_opts(self):
        descriptors = (
            ('action', Int8('appl_flg',
                        values={
                            'QUERY':pdu.ACP_MSG_PROV_NO_CHANGE,
                            'ACTIVATE':pdu.ACP_MSG_PROV_ACTIVATE,
                            'DEACTIVATE':pdu.ACP_MSG_PROV_DEACTIVATE,
                            'CHANGE':pdu.ACP_MSG_PROV_CHANGE,
                        })),
        )
        msg = insert_args_in_msg(descriptors,
                                {'action': 'QUERY'},
                                pdu.ProvUpd())
        self.assertEquals(pdu.ACP_MSG_PROV_NO_CHANGE, msg.appl_flg)
        self.assertEquals(
                'QUERY',
                insert_msg_in_args(descriptors, msg)['action'])

    def test_Int8_required(self):
        descriptors = (
            ('car_manufacturer_id',
                Int8('version.car_manufacturer', required=True)),
        )
        try:
            insert_args_in_msg(descriptors, {}, pdu.ProvUpd())
        except RequiredError:
            pass
        else:
            self.fail()

    def test_Int8_hex(self):
        descriptors = (
            ('car_manufacturer_id', Int8('version.car_manufacturer')),
            ('tcu_manufacturer_id', Int8('version.tcu_manufacturer')),
            ('major_hardware_release', Int8('version.major_hard_rel')),
            ('major_software_release', Int8('version.major_soft_rel')),
        )

        msg = insert_args_in_msg(descriptors,
                                {},
                                pdu.ProvUpd())
        self.assert_(msg.version is None)
        self.assert_(not insert_msg_in_args(descriptors, msg))

        msg = insert_args_in_msg(descriptors,
                                {'tcu_manufacturer_id': '0x82'},
                                pdu.ProvUpd())
        self.assertEquals(0x82, msg.version.tcu_manufacturer)
        self.assertEquals('130', insert_msg_in_args(descriptors, msg)['tcu_manufacturer_id'])

    def test_Time(self):
        descriptors = (
            ('start_time', Time('start_time')),
        )
        for rep in str(calendar.timegm((2009,1,2,3,4,5,0,0,0))),\
                   '2009-1-2 3:4:5',\
                   '2009-1-2 03:04:05':
            msg = insert_args_in_msg(descriptors,
                                    {'start_time': rep },
                                    pdu.ProvUpd())
            self.assertEquals(2009, msg.start_time.year)
            self.assertEquals(1, msg.start_time.month)
            self.assertEquals(2, msg.start_time.day)
            self.assertEquals(3, msg.start_time.hour)
            self.assertEquals(4, msg.start_time.minute)
            self.assertEquals(5, msg.start_time.second)
            self.assertEquals(
                str(calendar.timegm((2009,1,2,3,4,5,0,0,0))),
                insert_msg_in_args(descriptors, msg)['start_time'])

        try:
            args = {'start_time': 'foobar'}
            insert_args_in_msg(descriptors, args, msg)
        except FormatError:
            pass
        else:
            self.fail()

    def test_String(self):
        descriptors = (
            ('vin_number', String('vehicle_desc.vin')),
        )
        msg = insert_args_in_msg(descriptors,
                                {'vin_number': 'ABCDEF'},
                                pdu.ProvUpd())
        self.assertEquals('ABCDEF', msg.vehicle_desc.vin)
        self.assertEquals('ABCDEF', 
                          insert_msg_in_args(descriptors, msg)['vin_number'])

    def test_BinHex(self):
        descriptors = (
            ('authorization_key', BinHex('vehicle_desc.auth_key')),
        )

        msg = insert_args_in_msg(descriptors,
                                {'authorization_key': 'ABCDEF'},
                                pdu.ProvUpd())
        self.assertEquals(
            '\xAB\xCD\xEF',
            msg.vehicle_desc.auth_key)
        self.assertEquals(
            'ABCDEF',
            insert_msg_in_args(descriptors, msg)['authorization_key'])

        try:
            insert_args_in_msg(descriptors,
                              {'authorization_key': 'XXXX'},
                              pdu.ProvUpd())
        except FormatError:
            pass
        else:
            self.fail()

    def test_List_int(self):
        descriptors = (
            ('gps_sats', List('location.curr_gps(IEGPSRawData).satellites', int)),
        )
        msg = insert_args_in_msg(descriptors,
                                 {
                                     'gps_sats': '1,2,3'
                                 },
                                 pdu.TrackPos()
                                )
        self.assertEquals((1,2,3), msg.location.curr_gps.satellites)

        self.assertEquals(
            '1,2,3',
            insert_msg_in_args(descriptors, msg)['gps_sats'])

    def test_List(self):
        descriptors = (
            ('p', List('tcu_data.items', pdu.IETCUDataItem, (
                ('cod', (Int8, 'type')),
                ('val', (BinHex, 'data')),
                )
            )),
        )

        msg = insert_args_in_msg(descriptors,
                                {
                                    'p_cnt': '2',
                                    'p_0_cod' : '1',
                                    'p_0_val' : 'ABCDEF',
                                    'p_1_cod' : '2',
                                    'p_1_val' : '010203',
                                },
                                pdu.CfgUpd245())
        self.assertEquals(2, len(msg.tcu_data.items))
        self.assertEquals(1, msg.tcu_data.items[0].type)
        self.assertEquals('\xAB\xCD\xEF', msg.tcu_data.items[0].data)
        self.assertEquals(2, msg.tcu_data.items[1].type)
        self.assertEquals('\x01\x02\x03', msg.tcu_data.items[1].data)

        self.assertEquals(
            2,
            insert_msg_in_args(descriptors, msg)['p_cnt'])
        self.assertEquals(
            '1',
            insert_msg_in_args(descriptors, msg)['p_0_cod'])
        self.assertEquals(
            'ABCDEF',
            insert_msg_in_args(descriptors, msg)['p_0_val'])
        self.assertEquals(
            '2',
            insert_msg_in_args(descriptors, msg)['p_1_cod'])
        self.assertEquals(
            '010203',
            insert_msg_in_args(descriptors, msg)['p_1_val'])
