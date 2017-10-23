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
"""Console module client/server scripted services."""

from zope.interface import implements

from acpyd import service, script
from acpyd.protocol import ACPServerFactory, ACPClientFactory, SERVER_ROLE, CLIENT_ROLE
from acpyd.console import web, testdb
from acpyd.interfaces import BaseResultHandler

class CloseOnResultHandler(BaseResultHandler):
    """A test result handler that closes the service connection when the test
    passes or fails.
    This is the behavior for console scripts, on which after a script finishes,
    the connection must be closed."""
    def __init__(self, conn):
        self.conn = conn
    def passed(self, *args):
        """Closes the connection."""
        self.conn.close()
    def failed(self, *args):
        """Closes the connection."""
        self.conn.close()

class ACPTestServerFactory(ACPServerFactory):
    """A scripted server factory that registers a close on result handler when
    the connection is established."""
    def connected(self, conn):
        """Called when the connection is established. Registers a result handler
        to close the connection when the tests ends."""
        ACPServerFactory.connected(self, conn)
        if self.script_builder:
            script = self.script_builder()
            script.add_result_handler(CloseOnResultHandler(conn))
            conn.start_script(script,args=self.args)

class ACPTestClientFactory(ACPClientFactory):
    """A scripted client factory that registers a close on result handler when
    the connection is established."""
    def connected(self, conn):
        """Called when the connection is established. Registers a result handler
        to close the connection when the tests ends."""
        ACPClientFactory.connected(self, conn)
        if self.script_builder:
            script = self.script_builder()
            script.add_result_handler(CloseOnResultHandler(conn))
            conn.start_script(script,args=self.args)

class ConsoleService(service.BaseScriptedService):
    """A scripted service for the console which handles the server and client
    side of the test."""
    def __init__(self, *args, **kwargs):
        service.BaseScriptedService.__init__(self, *args, **kwargs)
        self.REPORT_CONFIG_WIDGET = 'ReportConfig'

    def get_web_ui(self):
        """Returns the web interface used to manage and interact with the
        service."""
        return lambda: web.MainPage(self, authorizator=self.authorizator)
    
    
    def _new_script_provider(self, script_dir, prefix):
        return script.ScriptProvider(
            script_dir,
            default_timeout=self.default_timeout,
            prefix=prefix
        )

    def _new_server(self, script_dir):
        """Creates a new scripted server using a ACPTestServerFactory.
        It also registers a test observer/logger/handler that stores events on
        a DB."""
        server = service.ScriptedServerService(
            self,
            SERVER_ROLE,
            self._new_script_provider(script_dir, '%s_' % SERVER_ROLE),
            ACPTestServerFactory(),
            self.config,
        )
        self._register_testdb_observer(server)
        return server

    def _new_client(self, script_dir):
        """Creates a new scripted client using a ACPTestClientFactory.
        It also registers a test observer/logger/handler that stores events on
        a DB."""
        client = service.ScriptedClientService(
            self,
            CLIENT_ROLE,
            self._new_script_provider(script_dir, '%s_' % CLIENT_ROLE),
            ACPTestClientFactory(),
            self.config,
        )
        self._register_testdb_observer(client)
        return client

    def _register_testdb_observer(self, service):
        """Registers a TestDbObserver on the given service to store test events
        on the DB."""
        testdb_observer = testdb.TestDbObserver(testdb.TestDbLogStore(self.testdb))
        service.add_observer(testdb_observer)
        service.add_result_handler(testdb_observer)
        service.add_logger(testdb_observer)
