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
from zope.interface import implements
from twisted.internet import defer, reactor
import twisted.trial.unittest as unittest
from twisted.web import client
from twisted.application import service, internet

from acp245 import log

from acpyd import config
config.TEMPLATE_DIR = '../templates'
config.STATIC_DIR = '../static'
config.ATHENA_JS_DIR = '../js'
config.SCRIPT_DIR = '../console_scripts'

from acpyd.console.service import ConsoleService
from acpyd.interfaces import ITestResultHandler
from acpyd.gateway.service import GatewayService
from nevow import appserver

import logging
logger = logging.getLogger()

import acpyd.test.compat

class ACPConsoleScriptTestCase(unittest.TestCase):
    def setUp(self):
        log.setlevel(0)
        self.b = ConsoleService(config={
            'allowed-ports':[13001],
            'server-enabled': True,
            'client-enabled': True
        })
        self.b.add_server(port=13001)
        self.b.add_client(port=13001)
        internet.TCPServer(
            12001,
            appserver.NevowSite(self.b.get_web_ui()())
        ).setServiceParent(self.b)
        self.b.set_client_script_dir('../console_scripts/')
        self.b.set_server_script_dir('../console_scripts/')
        self.b.set_client_script('client_1_connect_and_wait')
        self.b.set_server_script('server_1_connect_and_wait')
        self.b.startService()

    def tearDown(self):
        self.b.stop_server()
        self.b.stop_client()
        self.b.stopService()
        log.setlevel(0)

    def _test_scripts(self, script):
        return self._test_mixed_scripts(script, script)

    def _test_mixed_scripts(self, server_script, client_script):
        self.b.set_server_script('server_%s' % server_script)
        self.b.set_client_script('client_%s' % client_script)
        d = defer.Deferred()

        test = self
        class ResultHandler:
            implements(ITestResultHandler)
            def __init__(self):
                self.ok = 0
            def start(self, script):
                pass
            def failed(self, *args, **kwargs):
                try:
                    test.fail('Test failed: %s' % str(args))
                except:
                    if not d.called:
                        d.errback()
            def passed(self, *args, **kwargs):
                self.ok += 1
                if self.ok == 2:
                    d.callback(None)

        self.b.add_result_handler(ResultHandler())
        self.b.start_server()
        self.b.start_client()

        return d

class ACPConsoleScriptCustomTest(ACPConsoleScriptTestCase):
    def setUp(self):
        ACPConsoleScriptTestCase.setUp(self)
        self.b.set_client_script_dir('../custom_scripts/')
        self.b.set_server_script_dir('../custom_scripts/')


class ACPConsoleScriptBasicTest(ACPConsoleScriptTestCase):
    def test_scripts_load_server(self):
        return self._test_scripts('7_load_server')

    def test_scripts_configure(self):
        return self._test_scripts('2_configure')

    def test_scripts_track(self):
        return self._test_scripts('3_track')

    def test_scripts_activate_immo(self):
        return self._test_scripts('4_activate_immo')

    def test_scripts_deactivate_immo(self):
        return self._test_scripts('5_deactivate_immo')

    def test_scripts_keep_alive(self):
        return self._test_scripts('6_ka')

    def test_scripts_example_autoreply(self):
        return self._test_scripts('example_autoreply')

    def test_scripts_example_timer(self):
        return self._test_scripts('example_timer')

