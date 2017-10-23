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

class ACPConsoleGenTest(unittest.TestCase):
    def setUp(self):
        log.setlevel(0)
        self.b = ConsoleService(config={
            'allowed-ports':[13001],
            'server-enabled': True,
            'client-enabled': True
        })
        self.b.set_client_script_dir('../gen_console_scripts/')
        self.b.set_server_script_dir('../gen_console_scripts/')
        self.b.startService()

    def tearDown(self):
        try:
            self.b.stop_server()
            self.b.stop_client()
            return self.b.stopService()
        finally:
            log.setlevel(0)

    def test_gen_scripts(self):
        scripts = self.b.get_server_scripts() + self.b.get_client_scripts()
        names = set()
        for script in scripts:
            if script.name.startswith('server_'):
                name = script.name[len('server_'):]
            elif script.name.startswith('client_'):
                name = script.name[len('client_'):]
            names.add(name)

        d = defer.Deferred()
        for name in names:
            def create_cb(name):
                def cb(*args):
                    logger.debug('Starting test of script: %s' % name)
                    return self.__test_scripts(name)
                return cb
            d.addCallback(create_cb(name))
        d.callback(None)
        return d

    def __test_scripts(self, script):
        self.b.set_client_script('client_%s' % script)
        self.b.set_server_script('server_%s' % script)
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
                    d.errback()
            def passed(self, *args, **kwargs):
                self.ok += 1
                if self.ok == 2:
                    d.callback(None)

        logger.debug('Starting server/client')
        self.b.add_result_handler(ResultHandler())
        self.b.add_server(port=13001)
        self.b.start_server()
        self.b.add_client(port=13001)
        self.b.start_client()

        def cleanup(res):
            self.b.clear_handlers()
            ds = []
            d1 = self.b.stop_server()
            if d1:
                ds.append(d1)
            d2 = self.b.stop_client()
            if d2:
                ds.append(d2)
            if ds:
                d3 = defer.gatherResults(ds)
                d3.addCallback(lambda x: res)
                d3.addCallback(lambda x: res)
                return d3
            else:
                return res

        d.addBoth(cleanup)
        return d
