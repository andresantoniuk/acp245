"""
Reports test results and position information to an external ACP245 server.

At this moment this is intended to work with pyavld server and nothing else.
It's used so that pyavld stores position information on an emaps database so
it's possible to follow the positions sent/received during the test on emaps.

When a test starts the AvlReporter will send a fake message signaling an engine
start to the ACP245 server, and an engine stop when the test ends. This is
done so that, in emaps, a new travel is recorded for each test case.
"""
import logging
import time

from zope.interface import implements

from twisted.application import service, internet
from twisted.internet import protocol, task

from acp245.pdu import *

from acpyd.protocol import ACPProtocol
from acpyd.interfaces import *

log = logging.getLogger()

class ACPFactory(protocol.ReconnectingClientFactory):
    """Creates connections to an external ACP245 server, sending keepalives and
    ignoring received messages.
    The factory handles only a single connection.
    """
    protocol = ACPProtocol

    def __init__(self, conf):
        """Creates the factory with the given configuration.

        The configuration is a dict that must include the following keys:
            'server-ka': interval in seconds to send keepalives to the server.
        """
        self.proto = None
        self.conf = conf
        self.ka = None

    def connected(self, proto):
        """Called when a connection was established.
        The factory starts sending keepalives through the connection."""
        if self.proto:
            raise Exception('Too many connections from factory')
        self.resetDelay()
        self.proto = proto
        self.ka = task.LoopingCall(
            lambda: self.proto.send_msg(AlarmKA()),
        )
        self.ka.start(self.conf['server-ka'], now=False)

    def disconnected(self, proto_, reason_):
        """Called when a connection is closed.
        The factory stops sending keepalives."""
        if self.ka:
            self.ka.stop()
            self.ka = None
        self.proto = None

    def debug(self, msg, *args):
        """Writes a debug message."""
        log.debug(msg, *args)

    def error(self, msg, *args):
        """Writes an error message."""
        log.error(msg, *args)

    def msg_error(self, proto, e):
        """Called when there's an error with a received message."""

    def msg_received(self, proto, msg):
        """Called when a message is received."""

    def msg_sent(self, proto, msg, data):
        """Called when a message is sent."""

    def send_msg(self, msg):
        """Sends the message through a default protocol."""
        if self.proto:
            self.proto.send_msg(msg)

class AvlReporter(service.MultiService, BaseResultHandler, BaseTestObserver):
    """A service to send position information received during a test to an
    external ACP245 server."""
    implements(ITestResultHandler, ITestObserver)

    def __init__(self, conf):
        """Creates a new AVL reporter.

        The configuration is a dict that must include the following keys:
            'server-ka': interval in seconds to send keepalives to the server.
            'server-ip': the ACP245 external server IP.
            'server-port': the ACP245 external server port.
            'server-timeout': a timeout to establish the connection.
        """
        BaseResultHandler.__init__(self)
        BaseTestObserver.__init__(self)
        service.MultiService.__init__(self)
        self.conf = conf
        self.client_factory = None
        self.client = None
        self.in_progress = False

    def _close(self):
        """Closes the service, stopping all the connections."""
        if self.client:
            self.client.stopService()
            self.client = None
        if self.client_factory:
            self.client_factory.stopTrying()
            self.client_factory = None
    def _open(self):
        """Opens the service, creating a protocol factory and initiating a
        connection."""
        self._close()

        self.client_factory = ACPFactory(self.conf)
        self.client = internet.TCPClient(
            self.conf['server-ip'],
            self.conf['server-port'],
            self.client_factory,
            self.conf['server-timeout'],
        )
        self.client.startService()

    def started(self, role, host):
        """Called when the service has been started. Opens the connection to
        the server."""
        self._open()

    def stopped(self, role, host):
        """Called when the service has been stopped. Closes the connection to
        the server."""
        self._close()

    def start(self, script):
        """Called when a test has been started."""
        self.in_progress = False

    def failed(self, script, msg, *args):
        """Called when a test has failed.
        Sends a message indicating an engine off to the server."""
        if self.in_progress and self.client_factory:
            self.client_factory.send_msg(
                AlarmNotif(
                    vehicle_desc=IEVehicleDesc(
                        iccid=str(self.conf['device-id'])
                    ),
                    timestamp=IETimestamp(
                        time=time.time()
                    ),
                    breakdown_status = IEBreakdownStatus(source=[0, ACP_BKD_VEHICLE_OFF, 0])
                )
            )
        self.in_progress = False

    def passed(self, script, msg, *args):
        """Called when a test has passed.
        Sends a message indicating an engine off to the server."""
        if self.in_progress:
            self.client_factory.send_msg(
                AlarmNotif(
                    vehicle_desc=IEVehicleDesc(
                        iccid=str(self.conf['device-id'])
                    ),
                    timestamp=IETimestamp(
                        time=time.time()
                    ),
                    breakdown_status = IEBreakdownStatus(source=[0,ACP_BKD_VEHICLE_OFF,0])
                )
            )
        self.in_progress = False

    def received_msg(self, role, host, peer, msg, data):
        """Called when a message has been received during the test. If the
        message has location information, a new message will be sent to the
        server with this location information."""
        self._check_for_location(msg)

    def sent_msg(self, role, host, peer, msg, data):
        """Called when a message has been sent during the test."""

    def _check_for_location(self, msg):
        """Checks if the message includes location information. If it does, it
        sends an TrackPos message to the ACP245 server with the location
        information included on the message."""
        if (hasattr(msg, 'location') and
           msg.location is not None and
           msg.location.curr_gps is not None and
           msg.location.curr_gps.present):

            if not self.in_progress:
                breakdown_status = IEBreakdownStatus(source=[0,ACP_BKD_VEHICLE_ON,0])
            else:
                breakdown_status = None

            self.client_factory.send_msg(
                TrackPos(
                    vehicle_desc=IEVehicleDesc(
                        iccid=str(self.conf['device-id'])
                    ),
                    location=msg.location,
                    timestamp=IETimestamp(
                        time=time.time()
                    ),
                    breakdown_status=breakdown_status
                )
            )
            self.in_progress = True
