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
"""Services for acpyd.
This module provides classes that are used by acpyd interfaces to interact with
the rest of the system."""
import logging

from twisted.application import service, internet

from acpyd import testdb

from acpyd.interfaces import (
        ITestResultHandler,
        TestObserverMixin,
        LoggerMixin,
        ResultHandlerMixin,
        ObserverCollection
)

from acpyd.testobservers import (
        TestbenchLogObserver,
        TestbenchLogLogger,
        TestbenchLogResultHandler,
        PythonLogStore
)

# Register report adapters for testdb
from acpyd.testdb import kmlrep
kmlrep.register()
from acpyd.testdb import pdfrep
pdfrep.register()

class Authorizator(object):
    """A simple user authorizators that checks user/pass against an in-memory
    dictionary."""
    def __init__(self):
        self.users = {}

    def add_user(self, user, passwd):
        """Registers a user with the authorizator.
        @param user the user name
        @param passwd the user password
        """
        self.users[user] = passwd

    def authorize(self, resource_, user, passwd):
        """Returns True if the given user is allowed to access resource with the
        given password."""
        return ((passwd is not None) and
                (user is not None) and
                (self.users.get(user, None) == passwd))

class BaseScriptedService(
    TestObserverMixin,
    LoggerMixin,
    ResultHandlerMixin,
    service.MultiService):
    """A service that executes server and client scripts. It manages two
    scripted peer services, one for the client and one for the server.
    Each acpyd instance has one instance of a subclass of this class for each
    module (ie. gateway or console) which handles both the server and client."""
    def __init__(self,
                 host='', log=None,
                 script_dir='gateway_scripts/',
                 testdb_name='acpyd.gateway.service.testdb',
                 authorizator=None,
                 config=None,
                 reports_dir='reports/'):
        TestObserverMixin.__init__(self)
        LoggerMixin.__init__(self)
        ResultHandlerMixin.__init__(self)
        service.MultiService.__init__(self)

        if log is None:
            self.log = logging.getLogger()
        else:
            self.log = log

        if config is None:
            config = {}
        self.host = host
        self.config = config
        self.reports_dir=reports_dir
        self.authorizator = authorizator

        self.testdb = testdb.TestDb(dbname=testdb_name)
        
        #this is used for rendered page elements whose values need reflection to all connected peers
        self.liveObservers = {}

        self.add_observer(TestbenchLogObserver(PythonLogStore(self.log)))
        self.add_logger(TestbenchLogLogger(PythonLogStore(self.log)))
        self.add_result_handler(TestbenchLogResultHandler(PythonLogStore(self.log)))

        if self.server_enabled:
            self._server = self._new_server(script_dir)

        if self.client_enabled:
            self._client = self._new_client(script_dir)
    
    def _get_report_config(self):
        if 'report_config' not in self.config:
                self.config['report_config'] = {}
        return self.config['report_config']
    
    def _set_report_config(self, value):
        self.config['report_config'] = value
    
    report_config = property(_get_report_config, _set_report_config)
    
    def _get_server(self):
        """Gets the server scripted service."""
        if self.server_enabled:
            return self._server
        else:
            return None
    server = property(_get_server)

    def _get_client(self):
        """Gets the client scripted service."""
        if self.client_enabled:
            return self._client
        else:
            return None
    client = property(_get_client)

    def _get_write_script(self):
        """Gets if the service is allowed to write scripts."""
        return self.config.get('write-script', True)
    def _set_write_script(self, value):
        """Sets if the service is allowed to write scripts."""
        self.config['write-script'] = value
    write_script = property(_get_write_script, _set_write_script)

    def _get_default_timeout(self):
        """Returns the default timeout for service scripts."""
        return self.config.get('default-timeout', 0)
    def _set_default_timeout(self, value):
        """Sets the default timeout for service scripts."""
        self.config['default-timeout'] = value
    default_timeout = property(_get_default_timeout, _set_default_timeout)

    def _get_server_enabled(self):
        """Returns True if the server service is enabled."""
        return self.config.get('server-enabled', 0)
    def _set_server_enabled(self, value):
        """Sets if the server service is enabled."""
        self.config['server-enabled'] = value
    server_enabled = property(_get_server_enabled, _set_server_enabled)

    def _get_client_enabled(self):
        """Returns True if the client service is enabled."""
        return self.config.get('client-enabled', 0)
    def _set_client_enabled(self, value):
        """Sets if the client service is enabled."""
        self.config['client-enabled'] = value
    client_enabled = property(_get_client_enabled, _set_client_enabled)
    
    
    def getLiveObserver(self, liveObject):
        """get the observers collection that corresponds to the object"""
        className = liveObject.__class__.__name__
        if className not in self.liveObservers:
                self.liveObservers[className] = ObserverCollection()
        return self.liveObservers[className]
    
    def get_server_scripts(self):
        """Returns a list of scripts that the server can execute."""
        assert self.server_enabled
        return self.server.get_scripts()

    def get_server_script(self):
        """Returns the current server script."""
        assert self.server_enabled
        return self.server.get_script()

    def get_client_scripts(self):
        """Returns a list of scripts available for the server."""
        assert self.client_enabled
        return self.client.get_scripts()

    def get_client_script(self):
        """Returns the current client script."""
        assert self.client_enabled
        return self.client.get_script()

    def set_client_script_dir(self, script_dir):
        """Sets the directory from where the client loads its scripts."""
        assert self.client_enabled
        self.client.set_script_dir(script_dir)

    def set_server_script_dir(self, script_dir):
        """Sets the directory from where the server loads its scripts."""
        assert self.server_enabled
        self.server.set_script_dir(script_dir)

    def is_server_started(self):
        """Returns True if the server connection factory is started."""
        assert self.server_enabled
        return self.server.is_started()

    def is_client_started(self):
        """Returns True if the client connection factory is started."""
        assert self.client_enabled
        return self.client.is_started()

    def clear_server_script(self):
        """Stops and removes the server script. New connections will no longer
        use the current script."""
        assert self.server_enabled
        return self.server.clear_script()

    def clear_client_script(self):
        """Stops and removes the client script. New connections will no longer
        use the current script."""
        assert self.client_enabled
        return self.client.clear_script()

    def add_server(self, port=None):
        """Adds a new server that listens on the given port."""
        assert self.server_enabled
        self.server.add_service(port=port)

    def add_client(self, ip=None, port=None):
        """Adds a new client that connects to the given IP and port."""
        assert self.client_enabled
        self.client.add_service(ip=ip, port=port)

    def start_server(self):
        """Starts the server connection factory."""
        assert self.server_enabled
        self.server.start_peer()

    def stop_server(self):
        """Stops the server connection factory."""
        assert self.server_enabled
        return self.server.stop_peer()

    def start_client(self):
        """Starts the client connection factory."""
        assert self.client_enabled
        self.client.start_peer()

    def stop_client(self):
        """Stops the client connection factory."""
        """Stops the client."""
        assert self.client_enabled
        return self.client.stop_peer()

    def set_server_script(self, script, args=None, timeout=None, deferred=None, **info):
        """Sets the script to execute on the server."""
        assert self.server_enabled
        self.server.set_script(script,
                               args=args,
                               timeout=timeout,
                               deferred=deferred,
                               **info)

    def set_client_script(self, script, args=None, timeout=None, deferred=None, **info):
        """Sets the script to execute on the client."""
        assert self.client_enabled
        self.client.set_script(script,
                               args=args,
                               timeout=timeout,
                               deferred=deferred,
                               **info)

    def get_web_ui(self):
        """Called to get the web interface that handles this service. Subclasses
        must override this method and return their web interface."""
        raise NotImplementedError()

    def _new_server(self):
        """Returns a new instance of the server service. Subclasses must
        override this method and return the right instance."""
        raise NotImplementedError()

    def _new_client(self):
        """Returns a new instance of the client service. Subclasses must
        override this method and return the right instance."""
        raise NotImplementedError()

class ScriptedPeerService(
    TestObserverMixin,
    LoggerMixin,
    ResultHandlerMixin,
    service.Service):
    """A service that executes scripts against another peer. This is a base
    class for client or server script services."""

    def __init__(self,
                 bench,
                 role,
                 script_provider,
                 factory,
                 config):
        TestObserverMixin.__init__(self)
        LoggerMixin.__init__(self)
        ResultHandlerMixin.__init__(self)

        self.log = bench.log

        self.bench = bench
        self.role = role
        self.script = None
        self.script_provider = script_provider
        self.config = config
        self.factory = factory

        # notify events to bench oversvers
        self.add_result_handler(bench.result_handler)
        self.add_observer(bench.observer)
        self.add_logger(bench.logger)

        # receive events from script executing
        self.script_provider.add_result_handler(self.result_handler)
        self.script_provider.add_logger(self.logger)

        # receive events from the factory
        self.factory.add_logger(self.logger)
        self.factory.add_observer(self.observer)

    def set_script(self, script_name, args=None, timeout=None, deferred=None):
        """Sets the script to use on this service connections."""
        builder = lambda: self.script_provider.get_script(
            script_name,
            role=self.role,
            timeout=timeout,
            deferred=deferred,
            info=self.bench.report_config
        )
        self.script = builder()
        self.factory.set_script_builder(builder, args=args)

    def set_script_dir(self, script_dir):
        """Sets the directory where to load scripts from."""
        self.script_provider.set_script_dir(script_dir)

    def get_scripts(self):
        """Returns the scripts that are available for this service."""
        return self.script_provider.get_scripts()

    def get_script(self):
        """Returns the script used by this service connections."""
        return self.script

    def get_connections(self):
        """Returns a list of connections."""
        return self.factory.connections

    def start_peer(self):
        """Starts the connection factory."""
        self.factory.start_service()
        self.log.info(u'%s started' % self.role.capitalize())

    def stop_peer(self):
        """Stops the connection factory."""
        d = None
        if self.is_started():
            d = self.factory.stop_service()
            def stopped(*args):
                self.log.info(u'%s stopped' % self.role.capitalize())
            def errback(*args):
                self.log.error(u'%s stop failed' % self.role.capitalize())
            def remove(res):
                self.factory.remove_service()
                return res

            d.addCallbacks(stopped, errback)
            d.addBoth(remove)
        return d

    def is_started(self):
        """Returns True if the connection factory has been started."""
        return self.factory.is_service_started()

    def clear_script(self):
        """Stops and removes the current script, so it's no longer used on
        future connections."""
        return self.factory.clear_script()

    def _get_write_script(self):
        """Returns if the service is allowed to write scripts."""
        return self.config['write-script']
    def _set_write_script(self, value):
        """Sets if the service is allowed to write scripts."""
        self.config['write-script'] = value
    write_script = property(_get_write_script, _set_write_script)

    def _get_allowed_ports(self):
        """Returns the ports that the service can listen or connect to,
        depending if it's a client or server."""
        return self.config.get('allowed-ports', (12001,))
    def _set_allowed_ports(self, value):
        """Sets the ports that the service can listen or connect to,
        depending if it's a client or server."""
        self.config['allowed-ports'] = value
    allowed_ports = property(_get_allowed_ports, _set_allowed_ports)

    def _get_allowed_ips(self):
        """Gets the IPs that the service can listen or connect to,
        depending if it's a client or server."""
        return self.config.get('allowed-ips', (u'127.0.0.1',))
    def _set_allowed_ips(self, value):
        """Sets the IPs that the service can listen or connect to,
        depending if it's a client or server."""
        self.config['allowed-ips'] = value
    allowed_ips = property(_get_allowed_ips, _set_allowed_ips)

    def _get_ip(self):
        """Returns the default IP to listen or connect to."""
        return self.config.get('ip', u'127.0.0.1')
    def _set_ip(self, value):
        """Sets the default IP to listen or connect to."""
        self.config['ip'] = value
    ip = property(_get_ip, _set_ip)

    def _get_port(self):
        """Returns the default port to listen or connect to."""
        return self.config.get('port', 12001)
    def _set_port(self, value):
        """Sets the default port to listen or connect to."""
        self.config['port'] = value
    port = property(_get_port, _set_port)

class ScriptedClientService(ScriptedPeerService):
    """A service that starts a client that connects to a server and executes a
    script upon connecting."""
    def add_service(self, ip=None, port=None):
        if ip is None:
            ip = self.ip
        if port is None:
            port = self.port
        if port not in self.allowed_ports:
            raise Exception(
                u'Not allowed to open ACP connection to that port.'
                ' Allowed: %s' % ','.join(map(str,self.allowed_ports)))
        if ip not in self.allowed_ips:
            raise Exception(
                u'Not allowed to open ACP connection to that ip.'
                ' Allowed: %s' % ','.join(self.allowed_ips))
        self.factory.add_service(ip, port, internet.TCPClient)

class ScriptedServerService(ScriptedPeerService):
    """A service that starts a server that listens for clients."""
    def add_service(self, ip=None, port=None):
        if port is None:
            port = self.port
        if port not in self.allowed_ports:
            raise Exception(
                u'Not allowed to open ACP server on that port.'
                ' Allowed: %s' % ','.join(map(str,self.allowed_ports)))
        self.factory.add_service(port, internet.TCPServer)

class TestBenchService(service.MultiService):
    """A container for acpyd services.
    Scripted services, web servers and others are registered on this service to
    handle their lifecycle."""
