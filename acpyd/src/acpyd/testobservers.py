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
"""Basic test observers, result handlers and loggers that log into a log
store."""

from zope.interface import implements
from acpyd.interfaces import (
    ITestObserver, ITestLogger,
    ITestLogStore, ITestResultHandler,
    BaseTestObserver
)

# event names
SENT_MSG = u'sent_msg'
RECV_MSG = u'received_msg'
TEST_PASSED = u'test passed'
TEST_FAILED = u'test failed'
TEST_STARTED = u'test started'

class PythonLogStore(object):
    """A python log store stores log messages in a python Logger."""
    implements(ITestLogStore)

    def __init__(self, log):
        self._log = log

    def log(self, role, event, text, host=None, peer=None, msg=None, error=None,
            level=None, data=None):
        """Logs a new log line."""
        if error:
            log_msg = '[%s] %s: %s (%s)' % (
                (host is not None and '%s:%s' % (role, host)) or role,
                event, text, error)
        elif msg:
            log_msg = '[%s] %s: %s (%s)' % (
                (host is not None and '%s:%s' % (role, host)) or role,
                event, text, msg)
        else:
            log_msg = '[%s] %s: %s' % (
                (host is not None and '%s:%s' % (role, host)) or role,
                event, text)
        if level == 'debug':
            self._log.debug(log_msg)
        elif level == 'warn':
            self._log.warn(log_msg)
        elif level == 'error':
            self._log.error(log_msg)
        else:
            self._log.info(log_msg)

class TestbenchLogResultHandler():
    """A result handler that stores results in a test log store."""
    implements(ITestResultHandler)

    def __init__(self, log):
        self.log = ITestLogStore(log)

    def start(self, script):
        """Called when a test is started."""
        self.log.log(
            script.role,
            TEST_STARTED,
            'Test \'%s\' started' % script.name,
            level='all',
        )

    def failed(self, script, stack, msg, *args):
        """Called when a test fails."""
        if stack:
            self.log.log(
                script.role,
                TEST_FAILED,
                "TEST FAILED (%s line %s) at '%s' : %s" %  \
                    (script.name, stack[1], stack[3], str(msg) % args),
                level='all'
            )
        else:
            self.log.log(
                script.role,
                TEST_FAILED,
                "TEST FAILED (%s) : %s" %  (script.name, str(msg) % args),
                level='all'
            )

    def passed(self, script, msg, *args):
        """Called when a test passed."""
        self.log.log(
            script.role,
            TEST_PASSED,
            "TEST PASSED (%s): %s" % (script.name, msg % args),
            level='all'
        )

class TestbenchLogObserver(BaseTestObserver):
    """A test observer that logs events to a log store."""
    def __init__(self, log):
        self.log = ITestLogStore(log)

    def started(self, role, host):
        """Called when a service is started."""
        self.log.log(
            role,
            'started',
            'Service started %s:%d' % host,
            host=host,
            level='info'
        )

    def stopped(self, role, host):
        """Called when a service is stopped."""
        self.log.log(
            role,
            'stopped',
            'Service stopped %s:%d' % host,
            host=host,
            level='info'
        )

    def connected(self, role, host, peer):
        """Called when a service is connected."""
        self.log.log(
            role,
            'connected',
            'Connected to %s:%d' % peer,
            host=host,
            peer=peer,
            level='info'
        )

    def disconnected(self, role, host, peer):
        """Called when a service is disconnected."""
        self.log.log(
            role,
            'disconnected',
            'Disconnected from %s:%d' % peer,
            host=host,
            peer=peer,
            level='info'
        )

    def connection_failed(self, role, host, peer):
        """Called when a service connection fails."""
        self.log.log(
            role,
            'connection failed',
            'Connection failed to %s:%d' % peer,
            host=host,
            peer=peer,
            level='error'
        )

    def received_msg(self, role, host, peer, msg, data):
        """Called when a service receives a message."""
        self.log.log(
            role,
            RECV_MSG,
            'Received message "%s" from %s:%d' % ((msg.__class__.__name__,) + peer),
            host=host,
            peer=peer,
            level='info',
            msg=msg,
            data=data
        )

    def sent_msg(self, role, host, peer, msg, data):
        """Called when a service sends a message."""
        self.log.log(
            role,
            SENT_MSG,
            'Sent message "%s" to %s:%d' % ((msg.__class__.__name__,) + peer),
            host=host,
            peer=peer,
            level='info',
            msg=msg,
            data=data
        )

    def msg_error(self, role, host, e):
        """Called when a service receives a bad message."""
        self.log.log(
            role,
            'msg_error',
            'Bad message from %s:%d: "%s"' % (host + (e,)),
            host=host,
            level='error',
            error=unicode(e),
        )

    def io_error(self, role, host, e):
        """Called when there's an IO error on the service."""
        self.log.log(
            role,
            'io_error',
            'IO error from %s:%d: "%s"' % (host + (e,)),
            host=host,
            level='error',
            error=e,
        )

class TestbenchLogLogger:
    """A logger which logs to a test log store."""
    implements(ITestLogger)

    def __init__(self, log):
        self.log = ITestLogStore(log)

    def warn(self, role, msg, *args):
        """Called when a warn message is emitted."""
        self.log.log(
            role,
            'log',
            msg % args,
            level='warn')

    def error(self, role, msg, *args):
        """Called when an error message is emitted."""
        self.log.log(
            role,
            'log',
            msg % args,
            level='error')

    def info(self, role, msg, *args):
        """Called when an info message is emitted."""
        self.log.log(
            role,
            'log',
            msg % args,
            level='info')

    def debug(self, role, msg, *args):
        """Called when a debug message is emitted."""
        self.log.log(
            role,
            'log',
            msg % args,
            level='debug')
