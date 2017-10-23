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

import logging

from acpyd import config
config.TEMPLATE_DIR = '../templates'
config.STATIC_DIR = '../static'
config.ATHENA_JS_DIR = '../js'
config.SCRIPT_DIR = '../gateway_scripts'

import twisted.trial.unittest as unittest
from twisted.internet import defer, reactor
from twisted.application import service, internet

from acp245 import log
from acpyd.gateway.service import GatewayService
from nevow import appserver

import acpyd.test.compat

SERVER_PORT=13004
SERVER_HOST='localhost'

# Test cases
class ACPGwUiTest(unittest.TestCase):
    def test_ui(self):
        b = GatewayService(script_dir='../gateway_scripts/',
            config={
                'default_timeout': 5,
                'server-enabled': True,
                'client-enabled': True
            }
        )
        p = internet.TCPServer(
            SERVER_PORT,
            appserver.NevowSite(b.get_web_ui()())
        )
        p.setServiceParent(b)
        b.startService()
        d = defer.Deferred()

        def error(failure):
            d.errback(failure)
        def check_response(*args, **kwargs):
            # FIXME: find the right way to test the web UI.
            b.stop_server()
            b.stop_client()
            b.stopService()
            [x.cancel() for x in reactor.getDelayedCalls() if x != d]
            d.callback(None)

        req = client.getPage('http://%s:%s/gw/' % (SERVER_HOST, SERVER_PORT))
        req.addCallback(check_response)
        req.addErrback(error)
        return d

class ACPGwTestSec(unittest.TestCase):
    def test_ui(self):
        b = GatewayService(
            script_dir='../gateway_scripts/',
            config={
                'default_timeout': 5,
                'allowed-ports':[13001, 14001],
                'port': 13001,
                'server-enabled': True,
                'client-enabled': True
            }
        )
        p = internet.TCPServer(
            SERVER_PORT,
            appserver.NevowSite(b.get_web_ui()())
        )
        p.setServiceParent(b)
        b.startService()

        d = defer.Deferred()
        def cleanup(res):
            # FIXME: find the right way to test the web UI.
            b.stop_server()
            b.stop_client()
            b.stopService()
            return res

        d.addBoth(cleanup)
        def error(failure):
            d.errback(failure)

        def step1(*args):
            server_req('start?port=15001')\
                .addCallback(step2)\
                .addErrback(error)

        def step2(result):
            is_included(self, 'result=ERROR', result)
            server_req('start?port=14001')\
                .addCallback(step3)\
                .addErrback(error)

        def step3(result):
            are_eq(self, 'result=OK', result)
            client_req('start?port=15001')\
                .addCallback(step4)\
                .addErrback(error)

        def step4(result):
            is_included(self, 'result=ERROR', result)
            client_req('start?port=14001&ip=200.200.200.200')\
                .addCallback(step5)\
                .addErrback(error)

        def step5(result):
            is_included(self, 'result=ERROR', result)
            client_req('start?port=14001&ip=127.0.0.1')\
                .addCallback(step6)\
                .addErrback(error)

        def step6(result):
            is_included(self, 'result=OK', result)
            server_req('wait_for_connection')\
                .addCallback(step7)\
                .addErrback(error)

        def step7(result):
            is_included(self, 'result=OK&host=127.0.0.1', result)
            d.callback(None)

        step1()

        return d

class ACPGwTest(unittest.TestCase):
    def setUp(self):
        log.setlevel(0)
        self.b = GatewayService(script_dir='../gateway_scripts/',
            config={
                'default_timeout': 5,
                'server-enabled': True,
                'client-enabled': True
            }
        )
        p = internet.TCPServer(
            SERVER_PORT,
            appserver.NevowSite(self.b.get_web_ui()())
        )
        p.setServiceParent(self.b)
        try:
            self.b.startService()
        except:
            self.b.stop_server()
            self.b.stop_client()
            raise

    def tearDown(self):
        log.setlevel(0)
        ds = []
        d1 = self.b.stop_server()
        if d1: ds.append(d1)
        d2 = self.b.stop_client()
        if d2: ds.append(d1)
        self.b.stopService()
        self.b = None
        if ds:
            return defer.gatherResults(ds)

    def test_start_test_twice(self):
        d = defer.Deferred()
        def error(failure):
            d.errback(failure)

        def step0(*args):
            base_req('start_test?name=start_test_one')\
                .addCallback(step1)\
                .addErrback(error)

        def step1(result):
            are_eq(self, result, 'result=OK')
            base_req('stop_test')\
                .addCallback(step2)\
                .addErrback(error)

        def step2(result):
            are_eq(self, result, 'result=OK')
            base_req('start_test?name=start_test_two')\
                .addCallback(step3)\
                .addErrback(error)

        def step3(result):
            are_eq(self, result, 'result=OK')
            base_req('stop_test')\
                .addCallback(step4)\
                .addErrback(error)

        def step4(result):
            are_eq(self, result, 'result=OK')
            d.callback(None)

        step0()

        return d

    def test_start_test(self):
        d = defer.Deferred()
        def error(failure):
            d.errback(failure)

        def step0(*args):
            base_req('start_test')\
                .addCallback(step1)\
                .addErrback(error)

        def step1(result):
            are_eq(self, result, 'result=OK')
            server_req('start')\
                .addCallback(step2)\
                .addErrback(error)

        def step2(result):
            are_eq(self, result, 'result=OK')
            client_req('start')\
                .addCallback(step3)\
                .addErrback(error)

        def step3(result):
            are_eq(self, result, 'result=OK')
            server_req('wait_for_connection')\
                .addCallback(step4)\
                .addErrback(error)

        def step4(result):
            is_included(self, 'result=OK&host=127.0.0.1',
                        result)
            d1 = server_req('wait_for_disconnection')
            def stop_cli(*args):
                d2 = client_req('stop')
                defer.gatherResults([d1,d2])\
                    .addCallback(step5)\
                    .addErrback(error)
            reactor.callLater(1, stop_cli)

        def step5(results):
            self.assertEquals(2, len(results))
            is_included(self, 'result=OK&host=127.0.0.1',
                        results[0])
            are_eq(self, results[1], 'result=OK')
            base_req('stop_test')\
                .addCallback(step6)\
                .addErrback(error)

        def step6(result):
            are_eq(self, result, 'result=OK')
            d.callback(None)

        step0()

        return d

    def test_wait_for_disconnection(self):
        d = defer.Deferred()
        def error(failure):
            d.errback(failure)

        def step1(*args):
            server_req('start')\
                .addCallback(step2)\
                .addErrback(error)

        def step2(result):
            are_eq(self, result, 'result=OK')
            client_req('start')\
                .addCallback(step3)\
                .addErrback(error)

        def step3(result):
            are_eq(self, result, 'result=OK')
            server_req('wait_for_connection')\
                .addCallback(step4)\
                .addErrback(error)

        def step4(result):
            is_included(self, 'result=OK&host=127.0.0.1',
                        result)
            d1 = server_req('wait_for_disconnection')
            def stop_cli(*args):
                d2 = client_req('stop')
                defer.gatherResults([d1,d2])\
                    .addCallback(step5)\
                    .addErrback(error)
            reactor.callLater(1, stop_cli)

        def step5(results):
            self.assertEquals(2, len(results))
            is_included(self, 'result=OK&host=127.0.0.1',
                        results[0])
            are_eq(self, results[1], 'result=OK')
            d.callback(None)

        step1()

        return d

    def test_wait_for_connection(self):
        d = defer.Deferred()
        def error(failure):
            d.errback(failure)

        def step1(*args):
            server_req('start')\
                .addCallback(step2)\
                .addErrback(error)

        def step2(result):
            are_eq(self, result, 'result=OK')
            client_req('start')\
                .addCallback(step3)\
                .addErrback(error)

        def step3(result):
            are_eq(self, result, 'result=OK')
            server_req('wait_for_connection')\
                .addCallback(step4)\
                .addErrback(error)

        def step4(result):
            is_included(self, 'result=OK&host=127.0.0.1', result)
            d.callback(None)

        step1()

        return d

    def test_timeout(self):
        d = defer.Deferred()
        def error(failure):
            d.errback(failure)

        def step1(*args):
            server_req('start')\
                .addCallback(step2)\
                .addErrback(error)

        def step2(*args):
            client_req('start')\
                .addCallback(step3)\
                .addErrback(error)

        def step3(*args):
            client_req('wait_for_cfg_upd_245?timeout=1')\
                .addCallback(step4)\
                .addErrback(error)

        def step4(result):
            try:
                are_eq(self, 'result=TIMEOUT', result)
            except:
                d.errback()
            else:
                d.callback(None)

        step1()

        return d

    def test_disconnect_tcu_timeout(self):
        d = defer.Deferred()
        def error(failure):
            d.errback(failure)

        def step1(*args):
            server_req('start')\
                .addCallback(step2)\
                .addErrback(error)

        def step2(result):
            are_eq(self, result, 'result=OK')
            server_req('disconnect_tcu?timeout=1')\
                .addCallback(step3)\
                .addErrback(error)

        def step3(result):
            try:
                are_eq(self, 'result=TIMEOUT', result)
            except:
                d.errback()
            else:
                d.callback(None)

        step1()

        return d

    def test_disconnect_tcu_before_connect(self):
        d = defer.Deferred()
        def error(failure):
            d.errback(failure)

        def step1(*args):
            server_req('start')\
                .addCallback(step2)\
                .addErrback(error)

        def step2(result):
            are_eq(self, result, 'result=OK')
            d1 = server_req('disconnect_tcu')\
                    .addErrback(error)
            def start_cli(*args):
                d2 = client_req('start')
                defer.gatherResults([d1,d2])\
                    .addCallback(step3)\
                    .addErrback(error)
            reactor.callLater(1, start_cli)

        def step3(result):
            is_included(self, 'result=OK&host=127.0.0.1', result)
            d.callback(None)

        step1()

        return d

    def test_disconnect_tcu(self):
        d = defer.Deferred()
        def error(failure):
            d.errback(failure)

        def step1(*args):
            server_req('start')\
                .addCallback(step2)\
                .addErrback(error)

        def step2(*args):
            client_req('start')\
                .addCallback(step3)\
                .addErrback(error)

        def step3(result):
            are_eq(self, result, 'result=OK')
            server_req('disconnect_tcu')\
                .addCallback(step4)\
                .addErrback(error)

        def step4(result):
            is_included(self, 'result=OK&host=127.0.0.1', result)
            d.callback(None)

        step1()

        return d

    def test_send_prov_upd_with_hdr(self):
        d = defer.Deferred()
        def error(failure):
            d.errback(failure)

        def step1(*args):
            server_req('start')\
                .addCallback(step2)\
                .addErrback(error)

        def step2(result):
            is_included(self, { 'result': 'OK' }, result)
            client_req('start')\
                .addCallback(step3)\
                .addErrback(error)

        def step3(result):
            are_eq(self, result, 'result=OK')
            d1 = client_req('wait_for_prov_upd',
                            {
                                'vin_number':'DCBA',
                                'hdr_msg_ctrl': '0',
                                'hdr_version': '1',
                                'hdr_test_flag': '1'
                            }
            )
            d2 = server_req('send_prov_upd',
                            {
                                'vin_number':'ABCD',
                                'hdr_msg_ctrl': 'RESP_EXP',
                                'hdr_version': '2',
                                'hdr_test_flag': '0'
                            }
            )
            defer.gatherResults([d1,d2])\
                .addCallback(step4)\
                .addErrback(error)

        def step4(results):
            self.assertEquals(2, len(results))
            # check server result
            is_included(self,
                        {
                            'result': 'OK',
                            'vin_number':'DCBA',
                            'hdr_msg_ctrl': '0',
                            'hdr_version': '1',
                            'hdr_test_flag': '1'
                        },
                        results[1]
            )
            # check client result
            is_included(self,
                        {
                            'result': 'OK',
                            'vin_number':'ABCD',
                            'hdr_msg_ctrl': 'RESP_EXP',
                            'hdr_version': '2',
                            'hdr_test_flag': '0'
                        },
                        results[0]
            )
            d.callback(None)

        step1()
        return d
    def test_send_prov_upd(self):
        d = defer.Deferred()
        def error(failure):
            d.errback(failure)

        def step1(*args):
            server_req('start')\
                .addCallback(step2)\
                .addErrback(error)

        def step2(result):
            is_included(self, { 'result': 'OK' }, result)
            client_req('start')\
                .addCallback(step3)\
                .addErrback(error)

        def step3(result):
            are_eq(self, result, 'result=OK')
            d1 = client_req('wait_for_prov_upd',
                            {'vin_number':'ABCD'}
            )
            d2 = server_req('send_prov_upd',
                            {'vin_number':'ABCD'}
            )
            defer.gatherResults([d1,d2])\
                .addCallback(step4)\
                .addErrback(error)

        def step4(results):
            self.assertEquals(2, len(results))
            # check server result
            is_included(self,
                        { 'result': 'OK', 'vin_number':'ABCD' },
                        results[1]
            )
            # check client result
            is_included(self,
                        { 'result': 'OK' },
                        results[0]
            )
            d.callback(None)

        step1()
        return d

    def test_send_cfg_upd_245(self):
        # d represents a result that is not yet available
        # the result will be available when someone calls d.callback(result) in
        # case of success, or d.errback() in case of failure.
        # this test will not end until d has a result.

        d = defer.Deferred()
        def error(failure):
            d.errback(failure)

        def step1(*args):
            server_req('start')\
                .addCallback(step2)\
                .addErrback(error)

        def step2(result):
            are_eq(self, result, 'result=OK')
            client_req('start')\
                .addCallback(step3)\
                .addErrback(error)

        def step3(result):
            are_eq(self, result, 'result=OK')
            server_req('wait_for_connection')\
                .addCallback(step4)\
                .addErrback(error)

        def step4(result):
            is_included(self, 'result=OK&host=127.0.0.1', result)
            self.assertSubstring('result=OK', result)
            client_req('wait_for_connection')\
                .addCallback(step5)\
                .addErrback(error)

        def step5(*args):
            d1 = client_req('wait_for_cfg_upd_245',
                            { 'error': '2' }
            )
            d2 = server_req('send_cfg_upd_245')
            defer.gatherResults([d1,d2])\
                .addCallback(step6)\
                .addErrback(error)

        def step6(results):
            self.assertEquals(2, len(results))
            is_included(self,
                        { 'result': 'OK' },
                        results[0]
            )
            is_included(self,
                        {
                            'result': 'OK',
                            'error': '2'
                        },
                        results[1]
            )
            d.callback(None)

        # perform the first step of the test case
        step1()

        # return the deferred result
        # the test will stop once this deferred result has
        # an actual result (by some code that calls either
        # d.callback() on success, or d.errback() on failure)
        return d


    def test_send_cfg_upd_245_reply_245(self):
        # d represents a result that is not yet available
        # the result will be available when someone calls d.callback(result) in
        # case of success, or d.errback() in case of failure.
        # this test will not end until d has a result.

        d = defer.Deferred()
        def error(failure):
            d.errback(failure)

        def step1(*args):
            server_req('start')\
                .addCallback(step2)\
                .addErrback(error)

        def step2(result):
            are_eq(self, result, 'result=OK')
            client_req('start')\
                .addCallback(step3)\
                .addErrback(error)

        def step3(result):
            are_eq(self, result, 'result=OK')
            server_req('wait_for_connection')\
                .addCallback(step4)\
                .addErrback(error)

        def step4(result):
            is_included(self, 'result=OK&host=127.0.0.1', result)
            self.assertSubstring('result=OK', result)
            client_req('wait_for_connection')\
                .addCallback(step5)\
                .addErrback(error)

        def step5(*args):
            d1 = client_req('wait_for_cfg_upd_245_reply_245',
                            {
                                'errors_cnt': 2,
                                'errors_0_type': '3',
                                'errors_0_data': 'ABCD',
                                'errors_0_code': '1',

                                'errors_1_type': '4',
                                'errors_1_data': '0102',
                                'errors_1_code': '2'
                            }
            )
            d2 = server_req('send_cfg_upd_245')
            defer.gatherResults([d1,d2])\
                .addCallback(step6)\
                .addErrback(error)

        def step6(results):
            self.assertEquals(2, len(results))
            is_included(self,
                        { 'result': 'OK' },
                        results[0]
            )
            is_included(self,
                        {
                            'result': 'OK',
                            'errors_cnt': '2',
                            'errors_0_type': '3',
                            'errors_0_data': 'ABCD',
                            'errors_0_code': '1',

                            'errors_1_type': '4',
                            'errors_1_data': '0102',
                            'errors_1_code': '2'
                        },
                        results[1]
            )
            d.callback(None)

        # perform the first step of the test case
        step1()

        # return the deferred result
        # the test will stop once this deferred result has
        # an actual result (by some code that calls either
        # d.callback() on success, or d.errback() on failure)
        return d

    def test_pop_alarm_notif(self):
        d = defer.Deferred()
        def error(failure):
            d.errback(failure)

        def step1(*args):
            server_req('start')\
                .addCallback(step2)\
                .addErrback(error)

        def step2(result):
            is_included(self,
                        { 'result': 'OK' },
                        result
            )
            client_req('start')\
                .addCallback(step3)\
                .addErrback(error)

        def step3(result):
            are_eq(self, result, 'result=OK')
            d1 = server_req('pop_alarm_notif')
            d2 = client_req('send_alarm_notif',
                            {
                                'tcu_manufacturer_id': 1,
                                'car_manufacturer_id': 2,
                                'curr_sats': '1,2,3',
                                'timestamp': 1237552220
                            }
            )
            defer.gatherResults([d1,d2])\
                .addCallback(step4)\
                .addErrback(error)

        def step4(results):
            self.assertEquals(2, len(results))
            # check server result
            is_included(self,
                        {
                            'result': 'OK',
                            'tcu_manufacturer_id':'1',
                            'car_manufacturer_id':'2',
                            'timestamp':'1237552220',
                            'curr_sats':'1,2,3',
                        },
                        results[0]
            )
            # check client result
            is_included(self,
                        {
                            'result': 'OK'
                        },
                        results[1]
            )
            d.callback(None)

        step1()
        return d

    def test_send_alarm_reply(self):
        d = defer.Deferred()
        def error(failure):
            d.errback(failure)

        def step1(*args):
            server_req('start')\
                .addCallback(step2)\
                .addErrback(error)

        def step2(result):
            is_included(self,
                        { 'result': 'OK' },
                        result
            )
            client_req('start')\
                .addCallback(step3)\
                .addErrback(error)

        def step3(result):
            are_eq(self, result, 'result=OK')
            d1 = client_req('pop_alarm_reply')
            d2 = server_req('send_alarm_reply',
                            {
                                'vin_number':'ABCD',
                                'tcu_manufacturer_id': '1'
                            }
            )
            defer.gatherResults([d1,d2])\
                .addCallback(step4)\
                .addErrback(error)

        def step4(results):
            self.assertEquals(2, len(results))
            # check server result
            is_included(self,
                        { 'result': 'OK' },
                        results[1]
            )
            # check client result
            is_included(self,
                        { 'result': 'OK' },
                        results[0]
            )
            d.callback(None)

        step1()
        return d

    def test_pop_alarm_ka(self):
        d = defer.Deferred()
        def error(failure):
            d.errback(failure)

        def step1(*args):
            server_req('start')\
                .addCallback(step2)\
                .addErrback(error)

        def step2(result):
            is_included(self,
                        { 'result': 'OK' },
                        result
            )
            client_req('start')\
                .addCallback(step3)\
                .addErrback(error)

        def step3(result):
            are_eq(self, result, 'result=OK')
            d1 = client_req('send_alarm_ka',
                            {
                                'vin_number':'ABCD',
                                'tcu_manufacturer_id': '1'
                            }
            )
            d2 = server_req('pop_alarm_ka')
            defer.gatherResults([d1,d2])\
                .addCallback(step4)\
                .addErrback(error)

        def step4(results):
            self.assertEquals(2, len(results))
            # check server result
            is_included(self,
                        {
                            'result': 'OK',
                            'vin_number':'ABCD',
                        }, results[1]
            )
            # check client result
            is_included(self,
                        {
                            'result': 'OK'
                        },
                        results[0]
            )
            d.callback(None)

        step1()
        return d

    def test_send_track_reply(self):
        d = defer.Deferred()
        def error(failure):
            d.errback(failure)

        def step1(*args):
            server_req('start')\
                .addCallback(step2)\
                .addErrback(error)

        def step2(result):
            is_included(self,
                        { 'result': 'OK' },
                        result
            )
            client_req('start')\
                .addCallback(step3)\
                .addErrback(error)

        def step3(result):
            are_eq(self, result, 'result=OK')
            d1 = client_req('pop_track_reply')
            d2 = server_req('send_track_reply',
                            {
                                'vin_number':'ABCD',
                                'tcu_manufacturer_id': '1'
                            }
            )
            defer.gatherResults([d1,d2])\
                .addCallback(step4)\
                .addErrback(error)

        def step4(results):
            self.assertEquals(2, len(results))
            # check server result
            is_included(self,
                        { 'result': 'OK' },
                        results[1]
            )
            # check client result
            is_included(self,
                        { 'result': 'OK' },
                        results[0]
            )
            d.callback(None)

        step1()
        return d

    def test_send_track_cmd(self):
        d = defer.Deferred()
        def error(failure):
            d.errback(failure)

        def step1(*args):
            server_req('start')\
                .addCallback(step2)\
                .addErrback(error)

        def step2(result):
            is_included(self,
                        { 'result': 'OK' },
                        result
            )
            client_req('start')\
                .addCallback(step3)\
                .addErrback(error)

        def step3(result):
            are_eq(self, result, 'result=OK')
            d1 = client_req('wait_for_track_cmd',
                            {
                                'vin_number':'ABCD',
                                'tcu_manufacturer_id': '1'
                            }
            )
            d2 = server_req('send_track_cmd',
                            {
                                'vin_number':'ABCD',
                                'tcu_manufacturer_id': '1'
                            }
            )
            defer.gatherResults([d1,d2])\
                .addCallback(step4)\
                .addErrback(error)

        def step4(results):
            self.assertEquals(2, len(results))
            # check server result
            is_included(self,
                        {
                            'result': 'OK',
                            'tcu_manufacturer_id':'1',
                            'vin_number':'ABCD'
                        },
                        results[1]
            )
            # check client result
            is_included(self,
                        {
                            'result': 'OK',
                            'tcu_manufacturer_id':'1',
                            'vin_number':'ABCD'
                        },
                        results[0]
            )
            d.callback(None)

        step1()
        return d


    def test_pop_track_pos(self):
        d = defer.Deferred()
        def error(failure):
            d.errback(failure)

        def step1(*args):
            server_req('start')\
                .addCallback(step2)\
                .addErrback(error)

        def step2(result):
            is_included(self, { 'result': 'OK' }, result)
            client_req('start')\
                .addCallback(step3)\
                .addErrback(error)

        def step3(result):
            are_eq(self, result, 'result=OK')
            d1 = server_req('pop_track_pos')
            d2 = client_req('send_track_pos',
                        { 
                            'result': 'OK',
                            'tcu_manufacturer_id':'1',
                            'car_manufacturer_id':'2',
                            'timestamp':'1237552220',
                            'curr_sats':'1,2,3',
                            'curr_lon':'10',
                            'curr_lat':'20',
                            'curr_velocity':'30',
                            'sources':'9,8,7',
                            'sensor':'10',
                            'data':'ABCDED',
                            'info_type':'1',
                            'info_data':'DECDAB',
                        }
            )
            defer.gatherResults([d1,d2])\
                .addCallback(step4)\
                .addErrback(error)

        def step4(results):
            self.assertEquals(2, len(results))
            # check server result
            is_included(self,
                        {
                            'result': 'OK',
                            'tcu_manufacturer_id':'1',
                            'car_manufacturer_id':'2',
                            'timestamp':'1237552220',
                            'curr_sats':'1,2,3',
                            'curr_lon':'10',
                            'curr_lat':'20',
                            'curr_velocity':'30',
                            'sources':'9,8,7',
                            'sensor':'10',
                            'data':'ABCDED',
                            'info_type':'1',
                            'info_data':'DECDAB',
                        },
                        results[0]
            )
            # check client result
            is_included(self,
                        { 'result': 'OK' },
                        results[1]
            )
            d.callback(None)

        step1()
        return d

    def test_send_prov_upd_twice(self):
        d = defer.Deferred()
        def error(failure):
            d.errback(failure)

        def step1(*args):
            server_req('start')\
                .addCallback(step2)\
                .addErrback(error)

        def step2(result):
            is_included(self, { 'result': 'OK' }, result)
            client_req('start')\
                .addCallback(step3)\
                .addErrback(error)

        def step3(result):
            are_eq(self, result, 'result=OK')
            d1 = client_req('wait_for_prov_upd',{'vin_number':'BARFOO'})
            d2 = server_req('send_prov_upd',{'vin_number':'BARFOO'})
            defer.gatherResults([d1,d2])\
                .addCallback(step4)\
                .addErrback(error)

        def step4(results):
            self.assertEquals(2, len(results))
            # check server result
            is_included(self,
                        { 'result': 'OK', 'vin_number':'BARFOO' },
                        results[1])
            # check client result
            is_included(self,
                        { 'result': 'OK' },
                        results[0])
            d1 = client_req('wait_for_prov_upd',{'vin_number':'FOOBAR'})
            d2 = server_req('send_prov_upd',{'vin_number':'FOOBAR'})
            defer.gatherResults([d1,d2])\
                .addCallback(step5)\
                .addErrback(error)

        def step5(results):
            self.assertEquals(2, len(results))
            # check server result
            is_included(self,
                        { 'result': 'OK', 'vin_number':'FOOBAR' },
                        results[1])
            # check client result
            is_included(self,
                        { 'result': 'OK' },
                        results[0])
            d.callback(None)

        step1()
        return d

    def test_send_func_cmd(self):
        d = defer.Deferred()
        def error(failure):
            d.errback(failure)

        def step1(*args):
            server_req('start')\
                .addCallback(step2)\
                .addErrback(error)

        def step2(result):
            is_included(self, { 'result': 'OK' }, result)
            client_req('start')\
                .addCallback(step3)\
                .addErrback(error)

        def step3(result):
            are_eq(self, result, 'result=OK')
            d1 = client_req('wait_for_func_cmd',
                            {
                                'vin_number':'ABCD',
                                'car_manufacturer_id': '0X08',
                                'tcu_manufacturer_id': '0x82'
                            }
            )
            d2 = server_req('send_func_cmd',
                            {
                                'vin_number':'ABCD',
                                'car_manufacturer_id': '0X08',
                                'tcu_manufacturer_id': '0x82'
                            }
            )
            defer.gatherResults([d1,d2])\
                .addCallback(step4)\
                .addErrback(error)

        def step4(results):
            self.assertEquals(2, len(results))
            # check server result
            is_included(self,
                        {
                            'result': 'OK',
                            'tcu_manufacturer_id':'130',
                            'car_manufacturer_id':'8'
                        },
                        results[1]
            )
            # check client result
            is_included(self,
                        { 'result': 'OK' },
                        results[0]
            )
            d.callback(None)

        step1()
        return d

    def test_send_func_cmd_twice(self):
        d = defer.Deferred()
        def error(failure):
            d.errback(failure)

        def step1(*args):
            server_req('start')\
                .addCallback(step2)\
                .addErrback(error)

        def step2(result):
            is_included(self, { 'result': 'OK' }, result)
            client_req('start')\
                .addCallback(step3)\
                .addErrback(error)

        def step3(result):
            are_eq(self, result, 'result=OK')
            d1 = client_req('wait_for_func_cmd',
                           {
                               'function_status':'ENABLED',
                               'entity_id': '128'
                           }
            )
            d2 = server_req('send_func_cmd',
                            {
                                'function_command':'ENABLE',
                                'entity_id': '128',
                                'timeout': 5
                            }
            )
            defer.gatherResults([d1,d2])\
                .addCallback(step4)\
                .addErrback(error)

        def step4(results):
            self.assertEquals(2, len(results))
            # check server result
            is_included(self,
                        { 'result': 'OK',
                         'function_status':'ENABLED',
                         'entity_id': '128' },
                        results[1]
            )
            # check client result
            is_included(self,
                        { 'result': 'OK' },
                        results[0]
            )

            d1 = client_req('wait_for_func_cmd',
                            {
                                'function_status':'DISABLED',
                                'entity_id': '128'
                            }
            )
            d2 = server_req('send_func_cmd',
                            {
                                'function_command':'DISABLE',
                                'entity_id': '128',
                                'timeout': 5
                            }
            )
            defer.gatherResults([d1,d2])\
                .addCallback(step5)\
                .addErrback(error)

        def step5(results):
            self.assertEquals(2, len(results))
            # check server result
            is_included(self,
                        {
                            'result': 'OK',
                            'function_status':'DISABLED',
                            'entity_id': '128'
                        },
                        results[1]
            )
            # check client result
            is_included(self,
                        { 'result': 'OK' },
                        results[0]
            )
            d.callback(None)

        step1()
        return d

    def test_send_func_cmd_abort(self):
        d = defer.Deferred()
        def error(failure):
            d.errback(failure)

        def step1(*args):
            server_req('start')\
                .addCallback(step2)\
                .addErrback(error)

        def step2(result):
            is_included(self, { 'result': 'OK' }, result)
            client_req('start')\
                .addCallback(step3)\
                .addErrback(error)

        def step3(result):
            are_eq(self, result, 'result=OK')
            d1 = server_req('send_func_cmd',
                            {
                                'function_command':'ENABLE',
                                'entity_id': '128',
                                'timeout': 5
                            }
            )
            def server_send_req(*args):
                # prev request will be aborted before the client connects
                d2 = server_req('send_func_cmd',
                                {
                                    'function_command':'DISABLE',
                                    'entity_id': '128',
                                    'timeout': 5
                                }
                )
                def client_wait(*args):
                    d3 = client_req('wait_for_func_cmd',
                                {
                                    'function_status':'ENABLED',
                                    'entity_id': '128',
                                    'timeout': 5
                                }
                    )
                    defer.gatherResults([d1, d2,d3])\
                        .addCallback(step4)\
                        .addErrback(error)
                reactor.callLater(1, client_wait)
            reactor.callLater(1, server_send_req)

        def step4(results):
            self.assertEquals(3, len(results))
            # check aborted server request result
            is_included(self,
                        { 'result': 'ERROR' },
                        results[0]
            )
            # check server result,
            is_included(self,
                        {
                            'result': 'OK',
                            'function_status':'ENABLED',
                            'entity_id': '128'
                        },
                        results[1]
            )
            # check client result
            # * it will be enabled since the client
            # queued the first message *
            is_included(self,
                        {
                            'result': 'OK' ,
                            'function_command':'ENABLE',
                            'entity_id': '128'
                        },
                        results[2]
            )
            d.callback(None)

        step1()
        return d

    def test_send_func_cmd_abort_before_starting(self):
        d = defer.Deferred()
        def error(failure):
            d.errback(failure)

        def step1(*args):
            server_req('start')\
                .addCallback(step2)\
                .addErrback(error)

        def step2(result):
            are_eq(self, result, 'result=OK')
            d1 = server_req('send_func_cmd',
                            {
                                'function_command':'ENABLE',
                                'entity_id': '128',
                                'timeout': 4
                            }
            )
            def server_send_req(*args):
                d2 = server_req('send_func_cmd',
                                {
                                    'function_command':'DISABLE',
                                    'entity_id': '128',
                                    'timeout': 5
                                }
                )
                def client_start(*args):
                    def client_connected(*args):
                        d3 = client_req('wait_for_func_cmd',
                                       {
                                           'function_status':'DISABLED',
                                           'entity_id': '128'
                                       }
                        )
                        defer.gatherResults([d1, d2,d3])\
                            .addCallback(step4)\
                            .addErrback(error)
                    client_req('start')\
                        .addCallback(client_connected)\
                        .addErrback(error)
                reactor.callLater(1, client_start)
            reactor.callLater(1, server_send_req)

        def step4(results):
            self.assertEquals(3, len(results))
            # check aborted server request result
            # FIXME: result should be error
            is_included(self,
                        { 'result': 'TIMEOUT' },
                        results[0]
            )
            # check server result, it should be disabled
            # since first script didn't had a chance to run
            is_included(self,
                        {
                            'result': 'OK',
                            'function_status':'DISABLED',
                            'entity_id': '128'
                        },
                        results[1]
            )
            # check client result
            is_included(self,
                        { 'result': 'OK' },
                        results[2]
            )
            d.callback(None)

        step1()
        return d

# helper functions
from twisted.web import client
import cgi
import urllib
def http_req(path, file, args):
    qs = urllib.urlencode(args)
    if '?' in file:
        file = '%s&%s' % (file, qs)
    else:
        file = '%s?%s' % (file, qs)
    return client.getPage('%s%s' % (path, file))

def base_req(url, args={}):
    return http_req('http://%s:%s/gateway/' % (SERVER_HOST, SERVER_PORT), url, args)

def server_req(url, args={}):
    return http_req('http://%s:%s/gateway/server/' % (SERVER_HOST, SERVER_PORT), url, args)

def client_req(url, args={}):
    return http_req('http://%s:%s/gateway/client/' % (SERVER_HOST, SERVER_PORT), url, args)

def are_eq(self, data_dict, qs):
    if isinstance(data_dict, str):
        data_dict = dict(cgi.parse_qsl(data_dict))
    qs_dict = dict(cgi.parse_qsl(qs))
    self.assertEquals(data_dict, qs_dict)

def is_included(self, data_dict, qs):
    if isinstance(qs, tuple) or isinstance(qs, list):
        qs = qs[0]
    if isinstance(data_dict, str):
        data_dict = dict(cgi.parse_qsl(data_dict))
    qs_dict = qs is not None and dict(cgi.parse_qsl(qs)) or {}
    for x in data_dict:
        if x in qs_dict:
            self.assertEquals(data_dict[x], qs_dict[x],
                              'Invalid result: %s != %s (%s)' \
                              % (data_dict[x], qs_dict[x], qs))
        else:
            self.fail('Key "%s" not in response: %s' % (x, qs_dict))
