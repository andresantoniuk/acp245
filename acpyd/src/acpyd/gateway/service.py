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

from acpyd import service, script
from acpyd.protocol import ACPServerFactory, ACPClientFactory, SERVER_ROLE, CLIENT_ROLE
from acpyd.gateway import web, testdb

class ACPGwServerFactory(ACPServerFactory):
    def set_script_builder(self, script_builder, args=None):
        ACPServerFactory.set_script_builder(self, script_builder, args=args)
        for conn in self.connections:
            conn.start_script(script_builder(),args=args)
    def connected(self, conn):
        ACPServerFactory.connected(self, conn)
        if self.script_builder:
            conn.start_script(self.script_builder(),args=self.args)

    def disconnected(self, conn, reason):
        ACPServerFactory.disconnected(self, conn, reason)
        self.script_builder = None

    def stop_service(self):
        d = ACPServerFactory.stop_service(self)
        self.script_builder = None
        return d

class ACPGwClientFactory(ACPClientFactory):
    def set_script_builder(self, script_builder, args=None):
        ACPClientFactory.set_script_builder(self, script_builder, args=args)
        for conn in self.connections:
            conn.start_script(script_builder(),args=args)

    def connected(self, conn):
        ACPClientFactory.connected(self, conn)
        if self.script_builder:
            conn.start_script(self.script_builder(),args=self.args)

    def disconnected(self, conn, reason):
        ACPClientFactory.disconnected(self, conn, reason)
        self.script_builder = None

    def stop_service(self):
        d = ACPClientFactory.stop_service(self)
        self.script_builder = None
        return d

class GatewayService(service.BaseScriptedService):

    def __init__(self, *args, **kwargs):
        service.BaseScriptedService.__init__(self, *args, **kwargs)

        self.testdb_observer = testdb.TestDbObserver(
            testdb.TestDbLogStore(self.testdb, self))
        self.current_test = None
        self.added = False

    def start_test(self, name):
        """Starts a new test.

        All the script/connection events that occur between this call
        and the next call to start_test or stop_test will be grouped
        under the provided test name
        """
        self.current_test = self.testdb.new_test(name=name, **self.report_config)
        if not self.added:
            self.added = True
            self.server.add_observer(self.testdb_observer)
            self.server.add_result_handler(self.testdb_observer)
            self.client.add_observer(self.testdb_observer)
            self.client.add_result_handler(self.testdb_observer)

    def stop_test(self):
        """Stops the current test, if any.

        All the script/connections events that occur between this
        call and the next call to start_test will be ignored.
        """
        self.server.remove_observer(self.testdb_observer)
        self.server.remove_result_handler(self.testdb_observer)
        self.client.remove_observer(self.testdb_observer)
        self.client.remove_result_handler(self.testdb_observer)
        self.added = False
        self.current_test = None

    def get_web_ui(self):
        return lambda: web.MainPage(self, authorizator=self.authorizator)

    def _new_script_provider(self, script_dir, prefix):
        return script.ScriptProvider(
            script_dir,
            default_timeout=self.default_timeout,
            prefix=prefix
        )

    def _new_server(self, script_dir):
        return service.ScriptedServerService(
            self,
            SERVER_ROLE,
            self._new_script_provider(script_dir, '%s_' % SERVER_ROLE),
            ACPGwServerFactory(),
            self.config
        )

    def _new_client(self, script_dir):
        return service.ScriptedClientService(
            self,
            CLIENT_ROLE,
            self._new_script_provider(script_dir, '%s_' % CLIENT_ROLE),
            ACPGwClientFactory(),
            self.config
        )
