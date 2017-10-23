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
"""ACP245 Protocol client/servers and factories for test server."""

import sys
import binascii
import traceback
import exceptions

from twisted.internet import protocol, defer
from acp245 import pdu

from acpyd.interfaces import (
    ITestScript,
    IAutoReplyManager,
    TestObserverMixin,
    LoggerMixin
)

# Roles are used as an id string to describe the peer that sent/recv a message
CLIENT_ROLE = u'client'
SERVER_ROLE = u'server'

class ACPProtocol(protocol.Protocol):
    """Receives and sends ACP 245 messages."""

    def __init__(self):
        self.prev_data = ''

    @property
    def peer(self):
        """Returns a (ip,port) tuple identifying the other side of the
        connection."""
        return (self.transport.getPeer().host, self.transport.getPeer().port)

    @property
    def peer_name(self):
        """Returns the hostname/IP of the other side of the connection."""
        return self.transport.getPeer().host

    @property
    def peer_port(self):
        """Returns the port of the other side of the connection."""
        return self.transport.getPeer().port

    @property
    def host(self):
        """Returns a (ip, port) tuple identifying our side of the connection."""
        return (self.transport.getHost().host, self.transport.getHost().port)

    @property
    def host_name(self):
        """Returns the hostname/IP of our side of the connection."""
        return self.transport.getHost().host

    @property
    def host_port(self):
        """Returns the port of our side of the connection."""
        return self.transport.getHost().port

    @property
    def log(self):
        """Returns a logger to write message associtated to this connection."""
        return self.factory

    def dataReceived(self, data):
        """Called by Twisted when data is received."""
        self.log.debug("%d < %s:%d - %s", self.host_port, self.peer_name,
                self.peer_port, binascii.b2a_hex(data))
        try:
            try:
                self.prev_data += data
                while self.prev_data:
                    msg, sz = pdu.msg_read(self.prev_data)
                    consumed = self.prev_data[:sz]
                    self.prev_data = self.prev_data[sz:]
                    self.msg_received(msg, consumed)
            except pdu.MessageError, e:
                if e.code != pdu.ACP_MSG_ERR_INCOMPLETE:
                    self.prev_data = ''
                    self.msg_error(e)
        except:
            self.log.error('Unhandled exception: %s', sys.exc_info())
            self.msg_error(None)

    def send_msg(self, msg):
        """Sends a message to the other side of the connection."""
        if not self.connected:
            raise Exception('peer is not connected')
        data = pdu.msg_write(msg)
        self.log.debug("%d > %s:%d - %s", self.host_port, self.peer_name,
            self.peer_port, binascii.b2a_hex(data))
        self.transport.write(data)
        self.msg_sent(msg, data)

    def close(self):
        """Closes the connection."""
        self.transport.loseConnection()

    def connectionMade(self):
        """Called by Twisted when a connection is established."""
        self.connected = True
        self.factory.connected(self)

    def connectionLost(self, reason):
        """Called by Twisted when a connection is lost.
        This calls the factory disconnected method."""
        self.connected = False
        self.factory.disconnected(self, reason)
        self._connectionLost(reason)
        protocol.Protocol.connectionLost(self, reason)

    def _connectionLost(self):
        """Override this method to perform custom steps when a connection is
        lost. The factory will be already notified (by calling disconnect) when
        this method is called.
        Default behavior does nothing."""

    def msg_error(self, e):
        """Called when the received message has an error.
        Calls the factory msg_error method.
        @param e the exception that signaled the error or None for an unknown
        error."""
        self.factory.msg_error(self, e)

    def msg_received(self, msg, data):
        """Called when a message is received.
        Calls the factory msg_received method.
        @param msg the parsed message.
        @param data the message in binary."""
        self.factory.msg_received(self, msg, data)

    def msg_sent(self, msg, data):
        """Called when a message is sent.
        Calls the factory msg_sent method.
        @param msg the parsed message.
        @param data the message in binary."""
        self.factory.msg_sent(self, msg, data)

class ACPStoreProtocol(ACPProtocol):
    """An ACP Protocol that maintains a queue of received ACP245 messages."""

    def __init__(self):
        ACPProtocol.__init__(self)
        self.messages = []
        self.errors = []

    def has_reply(self, msg):
        """Returns True if the queue contains a reply for the given message.
        @param msg the message to search the reply for."""
        for reply in self.messages:
            if reply.is_reply(msg):
                return True
        return False

    def pop_first(self, cls=None):
        """Removes and returns the first message in the queue with from the
        given class, or the first message of the queue if class is None. If the
        queue is empty, returns None.
        @param cls the class of the message to pop. (optional)"""
        if cls is None:
            if self.messages:
                return self.messages.pop(0)
        else:
            for i in range(len(self.messages)):
                if isinstance(self.messages[i], cls):
                    return self.messages.pop(i)
        return None

    def pop_reply(self, msg):
        """Removes and returns the first reply found for the given message. If
        no reply is found for the message, returns None.
        @param the message to pop the reply for."""
        for i in range(len(self.messages)):
            if self.messages[i].is_reply(msg):
                return self.messages.pop(i)
        return None

    def discard_all(self):
        """Removes all the messages from the message queue."""
        self.messages = []

    def pop_msg(self, idx=0):
        """Removes and returns the message from the queue at the given index. If
        no index is provided, removes the first message.
        @param idx the index (optional).
        @raises IndexError if the queue is empty or there's no message at the
        given index."""
        if self.messages:
            return self.messages.pop(idx)
        else:
            raise IndexError("No available messages")

    def connectionLost(self, reason):
        """Called by Twisted when the connection was lost.
        @param reason an error code or description of why the connection was
        lost."""
        try:
            ACPProtocol.connectionLost(self, reason)
            self.messages = []
            self.errors = []
        except:
            self.log.error('Unhandled exception: %s', sys.exc_info())

    def connectionMade(self):
        """Called by Twisted when the connection is established."""
        try:
            ACPProtocol.connectionMade(self)
        except:
            self.log.error('Unhandled exception: %s', sys.exc_info())

    def msg_received(self, msg, data):
        """Appends the received message to the message queue, and calls
        _msg_received(msg, data) for further processing."""
        try:
            ACPProtocol.msg_received(self, msg, data)
            self.messages.append(msg)
            self._msg_received(msg, data)
        except:
            self.log.error('Unhandled exception: %s', sys.exc_info())

    def _msg_received(self, msg, data):
        """Override this method to perform further processing of the received
        message after it was added to the queue."""

    def msg_error(self, e):
        """Appends to an error queue the received message error, and calls
        _msg_error(e)."""
        try:
            ACPProtocol.msg_error(self, e)
            self.errors.append(e)
            self._msg_error(e)
        except:
            self.log.error('Unhandled exception: %s', sys.exc_info())

    def _msg_error(self, reason):
        """Override this method to perform further processing of the error after
        it was added to the error queue."""

class ACPScriptProtocol(ACPStoreProtocol):
    """A script protocol is an store protocol which also handles the lifecycle
    of scripts that are started when the connection is established and aborted
    when the connection is lost."""

    def __init__(self):
        ACPStoreProtocol.__init__(self)
        self.script = None

    def _connectionLost(self, reason):
        """Tries to end the script by calling next(), or otherwise aborts the
        scripts."""
        if self.script:
            try:
                self.script.next()
            except exceptions.StopIteration:
                """Script ended successfuly"""
            else:
                self.script.abort()
                self.script = None

    def clear_script(self):
        """Aborts the script if any, and clears the current script so it doesn't
        get executed on a new connection."""
        if self.script:
            self.script.abort()
            self.script = None

    def start_script(self, script, args=None):
        """Starts the given script with the given arguments, and performs the
        first step of the script.
        If there's an script set for this protocol, it's cleared."""
        if self.script:
            self.clear_script()
        self.script = ITestScript(script)
        if args is None:
            self.script.start(self)
        else:
            self.script.start(self, args)
        try:
            self.script.next()
        except exceptions.StopIteration:
            """Script ended successfuly"""

    def _msg_received(self, msg, data):
        """Checks if an autoreply is configured for the received message on the
        current script, and if so, returns a reply automatically or ignores the
        message. If no autoreply is configured, performs the next step of the
        script by calling next()."""
        if self.script:
            autoreplies = IAutoReplyManager(self.script)
            if autoreplies is not None and autoreplies.has_autoreply(msg):
                # remove it from the store
                self.messages.pop()
                reply = autoreplies.get_autoreply(msg)
                if callable(reply):
                    try:
                        try:
                            reply = reply(msg)
                        except exceptions.StopIteration:
                            # Script ended succesfully.
                            if self.script.passed or self.script.failed:
                                self.script = None
                                reply = None
                            else:
                                # The autoreply callback function raised an
                                # StopIteration  by itself, and not as a by
                                # product of calling test_passed or
                                # test_failed, raise it.
                                raise
                    except:
                        # Unknown exception raised by callback, log and
                        # signal script that an exception was raised.
                        self.script.log.exception(
                            "Script Autoreply failure (%s)",
                            reply.__name__
                        )
                        try:
                            # signal the script
                            self.script.got_exception()
                        except exceptions.StopIteration:
                            # Script ended, clear the script.
                            self.script = None
                            reply = None

                if reply is not None:
                    self.log.debug('autoreplying %s',
                                   msg.__class__.__name__)
                    self.send_msg(reply)
                else:
                    self.log.debug('ignoring %s',
                                   msg.__class__.__name__)
            else:
                # No autoreply, execute script next step
                try:
                    self.script.next()
                except exceptions.StopIteration:
                    self.script = None

    def _msg_error(self, e):
        """An invalid message was received, execute script next step and let it
        handle the condition."""
        if self.script:
            try:
                self.script.next()
            except exceptions.StopIteration:
                self.script = None

    def get_script(self):
        """Return the current script."""
        return self.script

class ACPProtoFactoryMixin(TestObserverMixin, LoggerMixin):
    """An ACP245 Factory that signals other obsevers and loggers when an event
    occurrs or a message is logged."""
    protocol = ACPStoreProtocol
    def __init__(self, role):
        TestObserverMixin.__init__(self)
        LoggerMixin.__init__(self)

        self.role = role

        self._conns = []
        self.service = None
        self.port = 0
        self.script_builder = None
        self.args = None

    @property
    def connections(self):
        """Returns the connections created by the factory."""
        return self._conns

    # FIXME: The factory is somewhat ready to handle multiple connections, but
    # we only accept a single connection per server instance on the original
    # design. See if it's possible to handle multiple connection or if we have
    # problems on some other places of the code.
    def connected(self, conn):
        """Called when the connection is established.
        If a connection is in progress, it's closed.
        Notifies registered observers by calling connected."""
        if self.connections:
            # only allow one connected client
            self.close_conns()
        self._conns.append(conn)
        self.observer.connected(self.role, conn.host, conn.peer)

    def disconnected(self, conn, reason):
        """Called when a connection is disconnected.
        Notifies registered observers by calling disconnected."""
        if conn in self.connections:
            self._conns.remove(conn)
        self.observer.disconnected(self.role, None, conn.peer)

    def msg_received(self, conn, msg, data):
        """Called when a connection receives a message.
        Notifies registered observers by calling received_msg."""
        self.observer.received_msg(
            self.role,
            conn.host,
            conn.peer,
            msg,
            data
        )

    def msg_sent(self, conn, msg, data):
        """Called when a connection sends a message.
        Notifies registered observers by calling sent_msg."""
        self.observer.sent_msg(
            self.role,
            conn.host,
            conn.peer,
            msg,
            data
        )

    def msg_error(self, conn, e):
        """Called when a connection receives a bad message.
        Notifies registered observers by calling msg_error."""
        self.observer.msg_error(self.role, conn.host, conn.peer, e)

    def debug(self, msg, *args):
        """Called when a connection wants to write a debug message.
        Notifies registered loggers by calling debug."""
        self.logger.debug(self.role, msg, *args)

    def error(self, msg, *args):
        """Called when a connection wants to write a error message.
        Notifies registered loggers by calling error."""
        self.logger.error(self.role, msg, *args)

    def close_conns(self):
        """Closes all the connections opened by this factory."""
        for conn in list(self.connections):
            conn.close()

    # UGLY: The binding from factory to service smells bad. I think the idea is
    # to just keep a single reference on the users of the factory, instead of
    # having the service + the factory.
    def start_service(self):
        """Starts the service that uses this factory to accept new
        connections."""
        if self.is_service_started():
            raise Exception('Already started')
        self.service.startService()
        self.observer.started(self.role, ('localhost', self.port))

    def stop_service(self):
        """Stops the service that uses this factory to accept new
        connections."""
        self.close_conns()
        if self.is_service_started():
            d = self.service.stopService()
            if d is None:
                d = defer.succeed(None)
            def callback(result):
                self.observer.stopped(self.role, ('localhost', self.port))
            d.addCallback(callback)
            return d

    def is_service_started(self):
        """Returns True if the service is already started."""
        return (self.service is not None) and self.service.running

    def remove_service(self):
        """Stops and removes the service that uses this factory to accept new
        connections."""
        d = self.stop_service()
        if d is None:
            d = defer.succeed(None)
        def callback(result):
            self.service = None
        d.addCallback(callback)
        return d

    def clear_script(self):
        """Clears the script of all the factory connections."""
        for conn in self.connections:
            conn.clear_script()

    def set_script_builder(self, script_builder, args=None):
        """Sets a functionto call to create new scripts."""
        self.script_builder = script_builder
        self.args = args

class ACPServerFactory(ACPProtoFactoryMixin, protocol.ServerFactory):
    """An ACP245 Factory for servers."""
    protocol = ACPScriptProtocol
    def __init__(self):
        ACPProtoFactoryMixin.__init__(self, role=SERVER_ROLE)

    def add_service(self, port, transport):
        """Adds a new server service."""
        if self.service is not None:
            raise Exception('Only supports one %s service at a time' % self.role)
        self.service = transport(port, self)
        self.port = port

class ACPClientFactory(ACPProtoFactoryMixin, protocol.ClientFactory):
    """An ACP245 Factory for servers."""
    protocol = ACPScriptProtocol
    def __init__(self):
        ACPProtoFactoryMixin.__init__(self, role=CLIENT_ROLE)

    def add_service(self, ip, port, transport):
        """Adds a new client service."""
        if self.service is not None:
            raise Exception('Only supports one %s service at a time' % self.role)
        self.service = transport(ip, port, self, 60)
        self.port = port

    def disconnected(self, conn, reason):
        """Called when the client is disconnected. Removes the current service
        from the factory.
        By removing the service, the client service will be automatically
        stopped."""
        ACPProtoFactoryMixin.disconnected(self, conn, reason)
        self.remove_service()

    def clientConnectionFailed(self, conn, reason):
        """Called by Twisted when the client connection fails. Removes the
        current service from the factory.
        By removing the service, the client service will be automatically
        stopped."""
        self.observer.connection_failed(self.role, None, (conn.host, conn.port))
        self.remove_service()
