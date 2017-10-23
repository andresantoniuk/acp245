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
"""Base interfaces for acpyd."""
# Interfaces have not evolved in the best way, specially the ITestLogStore, a
# refactoring should be performed.

from zope.interface import Interface, Attribute, implements

class ITestScript(Interface):
    """A test script.

    For acpyd, a test script has a main function that is called when the script
    is started, and that can return a generator.

    After started, the next() function must be called so the script executes the
    first step of the main function. If the main function returns a
    generator, the function will stop at each yield statement, and
    next() function must be called to proceed. Otherwise, the script will end
    with a result after the first call to next().

    Results are notified to result handlers registered with the script.

    A script can have the following results:
        * passed: The test succeeded, either because it ran until it's end
        without errors or because it the script explicitely succeeded.
        * failed: There was an error on the test, an assertion failed, the
        test was explictely failed, or the test was stopped before it passed.
    """

    name = Attribute("""The script name""")
    description = Attribute("""A description of the tests performed by the
                            script""")
    script_args = Attribute("""The arguments used to start the script and
                            execute it's main function""")
    script_kwargs = Attribute("""The keyword arguments used to start the script
                              and execute it's main function""")
    passed = Attribute("""True if the test has passed""")
    failed = Attribute("""True if the test has failed""")
    running = Attribute("""True if the test has started and not yet ended""")
    log = Attribute("""A logger that sends messages to the script ITestLoggers""")

    def add_result_handler(result_handler):
        """Adds a handler for this script results.
        @param result_handler an ITestResultHandler."""

    def add_logger(logger):
        """Adds a logger for log messages emited by the script.
        @param logger an ITestLogger."""

    def start(conn, *args, **kwargs):
        """Starts the test script.

        Every script receives as a first argument the connection that caused
        this script to start executing.

        Additional args can be specified, and will be passed to the script main
        function.

        This function will not execute code from the main function, it will only
        initialize the script and set it up so you can start executing each of
        the script steps. To perform the first step, you must call the next()
        function.
        """

    def next():
        """Performs the next step of the script main function.
        @raises StopIteration if the script ends (either succeeding or
        failing)."""

    def abort():
        """Aborts the execution of the script. If the script has already been
        stopped, it does nothing."""

    def got_exception():
        """Called by a service that is executing a script when an exception
        ocurred while performing actions requested by the script.

        This function is rarely called, but is necessary when the script
        registers a timer or performs other actions that will be done on
        behalf of the script but not while executing the script main function.
        On those cases, this function can be called to notify the script of an
        error and make it abort with the last raised exception."""

    def test_failed(msg='failed', *args):
        """Fails the test."""

    def test_passed(msg='passed', *args):
        """Passes the test."""

class IEditableTestScript(Interface):
    """A script whose source code can be edited."""

    def get_source():
        """Returns the source code of the script."""

    def save_as(source, name):
        """Saves the current script using the new source under another name.
        This script name will be changed to the new name."""

    def update(source):
        """Saves the current script using the new source."""

class IAutoReplyManager(Interface):
    """A manager of auto replies for ACP245 messages."""
    def has_autoreply(msg):
        """Returns True if there's an autoreply available for the given
        message."""

    def get_autoreply(msg):
        """Returns the auto reply for the message.
        @return an ACP245 message or None."""

class ITestScriptTimer(Interface):
    """A timer used on test scripts."""
    def active():
        """Returns True if the timer associated function needs to be called in the future."""

    def start(interval=None):
        """Starts the timer if not active, otherwise reschedules the timer to
        run using the new interval."""

    def cancel():
        """Cancels the timer. active() will return false until a new start() is performed."""

class ITestScriptBenchService(Interface):
    """A set of services exported to a test script through the bench object."""
    def assert_equals(a, b, msg=None):
        """Asserts a and b are equals, or fails the test."""

    def assert_true(cond, msg=None):
        """Asserts cond is true or fails the test."""

    def assert_false(cond, msg=None):
        """Asserts cond is false or fails the test."""

    def test_failed(msg='failed', *args):
        """Fails the test."""

    def test_passed(msg='passed', *args):
        """Passes the test."""

    def periodic_timer(interval, f):
        """Registers a periodic timer that will execute every 'interval' seconds
        by calling function 'f'.
        @param interval a number of seconds between calls to f.
        @param f the function to call.
        @return an ITestTimer."""

    def timer(delay, f):
        """Registers a timer that will be called once, after 'delay' seconds,
        by calling function 'f'.
        @param delay seconds to wait before calling f
        @param f the function to call.
        @return an ITestTimer."""

    def cancel_timers():
        """Cancels all the timers registered by the test script."""

    def warn(role, msg, *args):
        """Signals a warning message from the test."""

    def error(role, msg, *args):
        """Signals an error message from the test."""

    def info(role, msg, *args):
        """Signals an info message from the test."""

    def debug(role, msg, *args):
        """Signals a debug message from the test."""

class ITestScriptProvider(Interface):
    """A test script provider."""
    def add_result_handler(result_handler):
        """Adds a result handler that will be added to every script returned by
        this provider when calling get_script."""

    def add_logger(logger):
        """Adds an script logger that will be added to every script returned by
        this provider when calling get_script."""

    def get_scripts():
        """Get all the scripts stored by this provider.
        @return a list of scripts."""

    def get_script(script_name, role='n/a', timeout=None, deferred=None):
        """Loads and returns the given script.
        @param role an identification string for the script runner.
        @param timeout a number in seconds that the script should be allowed to run.
        @param deferred a deferred to callback when the script passes, or errback
        when the script fails (either because of a failure, exception or
        timeout).
        @return the script."""

class ITestLogger(Interface):
    """A logger of test events."""
    def warn(role, msg, *args):
        """Logs a warning message emitted by 'role'."""

    def error(role, msg, *args):
        """Logs an error message emitted by 'role'."""

    def info(role, msg, *args):
        """Logs an info message emitted by 'role'."""

    def debug(role, msg, *args):
        """Logs a debug message emitted by 'role'."""

class ITestLogStore(Interface):
    """A store of test log lines."""

    def log(role, event, text, host=None, peer=None, msg=None, error=None,
            level=None, data=None):
        """Stores a script log line.
        @param role the role that emitted the log line.
        @param event the event to log.
        @param text the text of the message to log.
        @param host the (ip, port) of the service that emitted the log.
        @param peer the (ip, port) of the peer connected to the service that
        emitted the log.
        @param msg the message received or sent that caused the service to emit
        this log line, if any.
        @param error the error raised by the service that caused the service to
        emit this log line, if any.
        @param level the importanceof the log line
        @param data the binary representation of msg
        """

class BaseTestLogger(object):
    """Base class for test logger that does nothing.
    Subclasses may override only the necessary methods."""
    implements(ITestLogger)
    def warn(self, role, msg, *args):
        """Does nothing."""

    def error(self, role, msg, *args):
        """Does nothing."""

    def info(self, role, msg, *args):
        """Does nothing."""

    def debug(self, role, msg, *args):
        """Does nothing."""

class MultiLogger(object):
    """A logger that forwards messages to a list of loggers subscribed with this
    logger."""
    implements(ITestLogger)

    def __init__(self):
        self.loggers = []

    def add_logger(self, logger):
        """Adds a new logger to the list of loggers."""
        self.loggers.append(logger)

    def remove_logger(self, logger):
        """Removes a logger from the list of loggers."""
        self.loggers.remove(logger)

    def warn(self, role, msg, *args):
        """Sends the warn to every logger on the list of loggers."""
        for logger in self.loggers:
            logger.warn(role, msg, *args)

    def error(self, role, msg, *args):
        """Sends the error to every logger on the list of loggers."""
        for logger in self.loggers:
            logger.error(role, msg, *args)

    def info(self, role, msg, *args):
        """Sends the info to every logger on the list of loggers."""
        for logger in self.loggers:
            logger.info(role, msg, *args)

    def debug(self, role, msg, *args):
        """Sends the debug to every logger on the list of loggers."""
        for logger in self.loggers:
            logger.debug(role, msg, *args)

class LoggerMixin(object):
    """A mixin for classes that want to manage a set if subscribed loggers."""
    def __init__(self):
        self.logger = MultiLogger()

    def add_logger(self, logger):
        """Adds a new logger to the list of loggers."""
        self.logger.add_logger(logger)

    def remove_logger(self, logger):
        """Removes a logger from the list of loggers."""
        self.logger.remove_logger(logger)

class ITestResultHandler(Interface):
    """A test script result handler."""
    def start(script):
        """Called when the test has started.
        @param script the script being used for the test."""

    def failed(script, msg, *args):
        """Called when the test has failed.
        @param script the script being used for the test.
        @param msg the failure message
        @param *args interpolation tuple for msg."""

    def passed(script, msg, *args):
        """Called when the test has passed.
        @param script the script being used for the test.
        @param msg the pass message.
        @param *args interpolation tuple for msg."""

class BaseResultHandler(object):
    """Base class for result handlers that does nothing.
    Subclasses may override only the necessary methods."""
    implements(ITestResultHandler)
    def start(self, script):
        """Does nothing."""
    def failed(self, script, msg, *args):
        """Does nothing."""
    def passed(self, script, msg, *args):
        """Does nothing."""

class MultiResultHandler:
    """A result handler that forwards results to a list of handlers subscribed with this
    handler."""
    implements(ITestResultHandler)

    def __init__(self):
        self.handlers = []

    def add_handler(self, handler):
        """Adds a new result handler to the list of handlers."""
        self.handlers.append(handler)

    def remove_handler(self, handler):
        """Removes a handler from the list of handlers."""
        self.handlers.remove(handler)

    def clear_handlers(self):
        """Removes all the handlers from the list of handlers."""
        self.handlers = []

    def start(self, script):
        """Notifies all the handlers that the script has started."""
        for handler in self.handlers:
            handler.start(script)

    def failed(self, script, stack, msg, *args):
        """Notifies all the handlers that the script has failed."""
        for handler in self.handlers:
            handler.failed(script, stack, msg, *args)

    def passed(self, script, msg, *args):
        """Notifies all the handlers that the script has passed."""
        for handler in self.handlers:
            handler.passed(script, msg, *args)

class ResultHandlerMixin(object):
    """A mixin for classes that want to manage a set if subscribed result
    handlers."""
    def __init__(self):
        self.result_handler = MultiResultHandler()

    def add_result_handler(self, handler):
        """Adds a new result handler to the list of handlers."""
        self.result_handler.add_handler(handler)

    def remove_result_handler(self, handler):
        """Removes a handler from the list of handlers."""
        self.result_handler.remove_handler(handler)

    def clear_handlers(self):
        """Removes all the handlers from the list of handlers."""
        self.result_handler.clear_handlers()

class ITestObserver(Interface):
    """An observer of test progress events.
    This observer does not handles test results, which are handled by
    ITestResultHanlder."""

    def started(role, host):
        """Called when the 'role' service has started.
        @param role the role
        @param host the (ip, port) of the started service."""

    def stopped(role, host):
        """Called when the 'role' service has stopped.
        @param role the role
        @param host the (ip, port) of the started service."""

    def connected(role, host, peer):
        """
        Called when the peer connects.
        @param role the role played by the peer.
        @param host a tuple of ip/port
        @param peer a tuple of ip/port
        """

    def disconnected(role, host, peer):
        """
        Called when the peer disconnects.
        @param role the role played by the peer.
        @param host a tuple of ip/port
        @param peer a tuple of ip/port
        """

    def connection_failed(role, host, peer):
        """
        Called when a connection fails.
        @param role the role played by the peer.
        @param host a tuple of ip/port
        @param peer a tuple of ip/port
        """

    def received_msg(role, host, peer, msg, data):
        """
        Called when a message is received.
        @param role the role played by the peer.
        @param host a tuple of ip/port
        @param peer a tuple of ip/port
        @param msg the message
        @param data the message in binary
        """

    def sent_msg(role, host, peer, msg, data):
        """
        Called when a message is sent.
        @param role the role played by the peer.
        @param host a tuple of ip/port
        @param peer a tuple of ip/port
        @param msg the message
        @param data the message in binary
        """

    def msg_error(role, host, e):
        """
        Called when there's an error on a received message.
        @param role the role played by the peer.
        @param host a tuple of ip/port
        @param e the error
        """

    def io_error(role, host, e):
        """
        Called when there's an I/O error on the connection.
        @param role the role played by the peer.
        @param host a tuple of ip/port
        @param e the error
        """

class BaseTestObserver(object):
    """Base class for test observers that does nothing.
    Subclasses may override only the necessary methods."""
    implements(ITestObserver)

    def started(self, role, host):
        """Does nothing."""

    def stopped(self, role, host):
        """Does nothing."""

    def connected(self, role, host, peer):
        """Does nothing."""

    def disconnected(self, role, host, peer):
        """Does nothing."""

    def connection_failed(self, role, host, peer):
        """Does nothing."""

    def received_msg(self, role, host, peer, msg, data):
        """Does nothing."""

    def sent_msg(self, role, host, peer, msg, data):
        """Does nothing."""

    def msg_error(self, role, host, e):
        """Does nothing."""

    def io_error(self, role, host, e):
        """Does nothing."""

class MultiTestObserver(object):
    implements(ITestObserver)

    def __init__(self):
        self.observers = []

    def add_observer(self, observer):
        """Adds a new observer to the list of observers."""
        self.observers.append(observer)

    def remove_observer(self, observer):
        """Removes the observer from the list of observers."""
        self.observers.remove(observer)

    def started(self, role, host):
        """Notifies all the observers that the service has started."""
        for observer in self.observers:
            observer.started(role, host)

    def stopped(self, role, host):
        """Notifies all the observers that the service has stopped."""
        for observer in self.observers:
            observer.stopped(role, host)

    def connected(self, role, host, peer):
        """Notifies all the observers that the service has connected."""
        for observer in self.observers:
            observer.connected(role, host, peer)

    def disconnected(self, role, host, peer):
        """Notifies all the observers that the service has disconnected."""
        for observer in self.observers:
            observer.disconnected(role, host, peer)

    def connection_failed(self, role, host, peer):
        """Notifies all the observers that the service connection has failed."""
        for observer in self.observers:
            observer.connection_failed(role, host, peer)

    def received_msg(self, role, host, peer, msg, data):
        """Notifies all the observers that the service received a message."""
        for observer in self.observers:
            observer.received_msg(role, host, peer, msg, data)

    def sent_msg(self, role, host, peer, msg, data):
        """Notifies all the observers that the service sent a message."""
        for observer in self.observers:
            observer.sent_msg(role, host, peer, msg, data)

    def msg_error(self, role, host, e):
        """Notifies all the observers that the service received a bad message."""
        for observer in self.observers:
            observer.msg_error(role, host, e)

    def io_error(self, role, host, e):
        """Notifies all the observers that there was an IO error on the service."""
        for observer in self.observers:
            observer.io_error(role, host, e)

class TestObserverMixin(object):
    """A mixin for classes that want to manage a set if subscribed test
    observers."""
    def __init__(self):
        self.observer = MultiTestObserver()

    def add_observer(self, observer):
        """Adds a new observer to the list of observers."""
        self.observer.add_observer(observer)

    def remove_observer(self, observer):
        """Removes the observer from the list of observers."""
        self.observer.remove_observer(observer)


class ObserverCollection(object):
    """An object working as Subject of observer patter"""
    def __init__(self):
        self.observerCollection=[]

    def registerObserver(self, observer):
        self.observerCollection.append(observer);

    def unregisterObserver(self, observer):
        self.observerCollection.remove(observer)

    def notifyObservers(self, *args, **kwargs):
        for obj in self.observerCollection:
            obj.notify(*args, **kwargs)

class Observer(object):

    def __init__(self, observerCollection):
        self.observerCollection = observerCollection
        self.observerCollection.registerObserver(self)

    def notify(self, *args,  **kwargs):
        pass

    def unregister(self):
        self.observerCollection.unregisterObserver(self)

class Observable(object):
    
    def __init__(self, observerCollection):
        self.observerCollection=observerCollection

    def sendNotify(self, *args, **kwargs):
        self.observerCollection.notifyObservers(*args, **kwargs)


