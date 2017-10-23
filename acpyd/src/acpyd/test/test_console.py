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

class ACPConsoleUiTest(unittest.TestCase):
    def test_ui(self):
        b = ConsoleService(config={
            'allowed-ports':[13001],
            'server-enabled': True,
            'client-enabled': True,
            'write-script': True,
        })
        b.add_server(port=13001)
        b.add_client(port=13001)
        internet.TCPServer(
            12001,
            appserver.NevowSite(b.get_web_ui()())
        ).setServiceParent(b)
        b.set_client_script_dir('../console_scripts/')
        b.set_server_script_dir('../console_scripts/')
        b.set_client_script('client_1_connect_and_wait')
        b.set_server_script('server_1_connect_and_wait')
        b.startService()

        d = defer.Deferred()

        def cleanup(*args):
            # FIXME: find the right way to test the web UI.
            b.stop_server()
            b.stop_client()
            b.stopService()
            [x.cancel() for x in reactor.getDelayedCalls() if x != d]

        def error(failure):
            cleanup()
            d.errback(failure)

        def check_response(*args, **kwargs):
            cleanup()
            d.callback(None)

        req = client.getPage('http://localhost:12001/')
        req.addCallback(check_response)
        req.addErrback(error)
        return d

