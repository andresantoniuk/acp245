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
"""A class library to build async test scritps for the ACP245 test server."""

import logging
import os
import random
import sys
import time
import traceback
import types
import types

from zope.interface import implements
from twisted.internet import reactor, defer
from twisted.python import failure, components

import acp245.stdmsg
import acp245.pdu

import acpyd.args
from acpyd.restricted import restricted_exec
from acpyd.flatten import FLATTENERS
from acpyd.interfaces import (
        ITestScript,
        IEditableTestScript,
        IAutoReplyManager,
        ITestScriptTimer,
        ITestScriptBenchService,
        ITestScriptProvider,
        ITestResultHandler,
        MultiResultHandler,
        MultiLogger
)

# FIXME: Change StopIteration to use a specific exception when a script is
# passed or failed by calling test_passed or test_failed.

def _find_tb(filename):
    """Searches the exception traceback for the first line in the traceback that
    belongs to filename. Returns the traceback at that line."""
    tbs = traceback.extract_tb(sys.exc_info()[2])
    for tb in tbs:
        if tb[0].endswith(filename):
            return tb
    return tbs[-1]

def _find_stack(filename):
    """Searches the current stack for the first line that belongs to filename.
    Returns the stack at that point. If no line in the stack belongs to
    filename, returns None."""
    tbs = traceback.extract_stack()
    for tb in tbs:
        if tb[0].endswith(filename):
            return tb
    return None

class TestScriptTimer:
    """A timer for test scripts."""
    implements(ITestScriptTimer)
    def __init__(self, script, interval, func, periodic=False, start=True):
        self.script = script
        self.func = func
        self.interval = interval
        self.periodic = periodic
        if start:
            self.timer = reactor.callLater(self.interval, self.__cb)
        else:
            self.timer = None

    def active(self):
        """Returns True if the timer associated function needs to be called in the future."""
        return self.timer is not None and self.timer.active()

    def start(self, interval=None):
        """Starts the timer if not active, otherwise reschedules the timer to
        run using the new interval."""
        if self.timer is not None and self.timer.active():
            self.timer.cancel()
        if interval:
            self.interval = interval
        self.timer = reactor.callLater(self.interval, self.__cb)
        return self

    def cancel(self):
        """Cancels the timer. active() will return false until a new start() is performed."""
        if self.timer is not None:
            self.timer.cancel()
            self.timer = None
        return self

    def __cb(self, *args, **kwargs):
        """Calls the callback. If the timer is periodic, reschedules the timer
        for the next interval."""
        self.timer = None
        # TODO, select a python version so we can use try/except/finally...
        try:
            try:
                self.func(*args, **kwargs)
            except StopIteration:
                if not self.script.passed and not self.script.failed:
                    raise
        except:
            self.script.log.exception(
                "Script timer failure (%s)",
                self.func.__name__
            )
            self.script.got_exception()
        finally:
            if self.periodic and self.script.running:
                self.start()

class TestScriptEnvironment(object):
    """
    Bench for executing test scripts.
    An instance of this class is exported under the name 'bench' to every
    script.
    """
    implements(ITestScriptBenchService)

    def __init__(self, script, result_handler, logger):
        self.__script = script
        self.__logger = logger
        self.__result_handler = result_handler
        self.__timers = []

    def assert_equals(self, a, b, msg=None):
        """Asserts a and b are equals, or fails the test."""
        if ((isinstance(a, tuple) or isinstance(a, list)) and
            (isinstance(b, tuple) or isinstance(b, list))):
            # usability, make tuples and list equals
            a = tuple(a)
            b = tuple(b)
        if not (a == b):
            self.test_failed(msg or "Expected '%s' was '%s'" % (b, a))

    def assert_true(self, cond, msg=None):
        """Asserts cond is true or fails the test."""
        if not cond:
            self.test_failed(msg or 'Assertion failed')

    def assert_false(self, cond, msg=None):
        """Asserts cond is false or fails the test."""
        if cond:
            self.test_failed(msg or 'Assertion failed')

    def test_failed(self, msg='failed', *args):
        """Fails the test."""
        self.__script.test_failed(msg=msg, *args)
        raise StopIteration()

    def test_passed(self, msg='passed', *args):
        """Passes the test."""
        self.__script.test_passed(msg=msg, *args)
        raise StopIteration()

    def periodic_timer(self, interval, f, start=True):
        """Registers a periodic timer that will execute every 'interval' seconds
        by calling function 'f'.
        @param interval a number of seconds between calls to f.
        @param f the function to call.
        @return an ITestTimer."""
        if not isinstance(interval, int) and not isinstance(interval, float):
            raise ValueError('Timer interval (first argument) must be a '\
                             'number of seconds')
        if not callable(f):
            raise ValueError('Timer second argument must be a function')
        timer = TestScriptTimer(self.__script, interval, f,
            periodic=True, start=start)
        self.__timers.append(timer)
        return timer

    def timer(self, interval, f, start=True):
        """Registers a timer that will be called once, after 'delay' seconds,
        by calling function 'f'.
        @param delay seconds to wait before calling f
        @param f the function to call.
        @return an ITestTimer."""
        if not isinstance(interval, int) and not isinstance(interval, float):
            raise ValueError('Timer interval (first argument) must be a '\
                             'number of seconds')
        if not callable(f):
            raise ValueError('Timer second argument must be a function')
        timer = TestScriptTimer(self.__script, interval, f,
            periodic=False, start=start)
        self.__timers.append(timer)
        return timer

    def cancel_timers(self):
        """Cancels all the timers registered by the test script."""
        for timer in [x for x in self.__timers if x.active()]:
            timer.cancel()
        self.__timers = []

    def warn(self, msg, *args):
        """Signals a warning message from the test."""
        if self.__logger is not None:
            self.__logger.warn(self.__script.role, msg, *args)

    def error(self, msg, *args):
        """Signals an error message from the test."""
        if self.__logger is not None:
            self.__logger.error(self.__script.role, msg, *args)

    def info(self, msg, *args):
        """Signals an info message from the test."""
        if self.__logger is not None:
            self.__logger.info(self.__script.role, msg, *args)

    def debug(self, msg, *args):
        """Signals a debug message from the test."""
        if self.__logger is not None:
            self.__logger.debug(self.__script.role, msg, *args)

class ReadMsgDict:
    """Maps binary stored messages to their respective parsed object."""
    def __init__(self, msgs):
        """msgs is a dictionary of msg_name->msg_hex_data"""
        self.__msgs = msgs
    def __getitem__(self, item):
        """Returns the acp245 PDU associated with the given item by interpreting
        the associated msg_hex_data."""
        msg = self.__msgs[item]
        return acp245.pdu.msg_read(hex=msg)[0]

class Result:
    """A marker adaptor for script results."""
    def __init__(self, value):
        self.value = value

def as_int(v):
    """Transforms a tuple of ints between 0 and 255, or a binary string, into an
    int, assming each tuple item is a byte.
    >>> as_int((1,)) # 0x1
    1  (0x1
    >>> as_int((1,2)) #0x12
    18
    >>> as_int((1,2,3)) #0x123
    291
    >>> as_int('\\x01\\x02') #0x12
    18
    """
    if isinstance(v, tuple) or isinstance(v, list):
        return reduce(lambda x,y: x << 8 | y, v)
    else:
        return int(v)

def as_str(v):
    """Transforms an tuple into a string by appending the items.
    >>> as_str((1,))
    '\\x01'
    >>> as_str((1,2))
    '\\x01\\x02'
    >>> as_str(('\\x01','\\x02'))
    '\\x01\\x02'
    """
    if isinstance(v, tuple) or isinstance(v, list):
        return ''.join([x if isinstance(x,basestring) else chr(x) for x in v])
    else:
        return str(v)

class AutoReplyManager(object):
    """A manager of auto replies for ACP245 messages."""
    implements(IAutoReplyManager)

    def __init__(self):
        self._autoreplies = {}

    def has_autoreply(self, msg):
        """Returns True if there's an autoreply available for the given
        message."""
        for p, reply in self._autoreplies.items():
            if type(p) in (types.FunctionType, types.MethodType):
                return p(msg)
            elif isinstance(msg, p):
                return True
        return False

    def get_autoreply(self, msg):
        """Returns the auto reply for the message.
        @return an ACP245 message or None."""
        for p, reply in self._autoreplies.items():
            if type(p) in (types.FunctionType, types.MethodType):
                if p(msg):
                    return reply
            elif isinstance(msg, p):
                return reply
        return None

    def __setitem__(self, name, value):
        """Overrides x[y] to registers a new autoreply.
        >>> from acp245 import pdu
        >>> s = AutoReplyManager()
        >>> s[pdu.AlarmKA] = pdu.AlarmKAReply()
        >>> s.has_autoreply(pdu.AlarmKA())
        True
        """
        self._autoreplies[name] = value

    def __getitem__(self, name):
        """Overrides x[y] to return the registered autoreply.
        >>> from acp245 import pdu
        >>> s = AutoReplyManager()
        >>> s[pdu.AlarmKA] = pdu.AlarmKAReply()
        >>> s[pdu.AlarmKA]
        AlarmKA()
        """
        return self._autoreplies[name]

    def __delitem__(self, name):
        """Overrides del x[y] to unregister an autoreply.
        >>> from acp245 import pdu
        >>> s = AutoReplyManager()
        >>> s[pdu.AlarmKA] = pdu.AlarmKAReply()
        >>> del s[pdu.AlarmKA]
        >>> s.has_autoreply(pdu.AlarmKA())
        False
        """
        del self._autoreplies[name]

class TestResult(object):
    """A test result."""
    PASSED = 1
    FAILED = 2
    def __init__(self, result, msg, args=None, stack=None):
        self.result = result
        self.msg = msg
        self.args = args or []
        self.stack = stack

    @property
    def passed(self):
        """Returns True if the test has passed."""
        return self.result == TestResult.PASSED

    @property
    def failed(self):
        """Returns True if the test has failed."""
        return self.result == TestResult.FAILED

class TestScript(object):
    """Base class for scripts."""
    implements(ITestScript, IEditableTestScript)
    def __init__(self,
                 script_dir,
                 name,
                 role='n/a',
                 timeout=0,
                 log=None,
                 deferred=None,
                 info={}):

        self.result_handler = MultiResultHandler()
        self.logger = MultiLogger()
        self.autoreplies = AutoReplyManager()

        self._name = name
        self._short_name = name
        self._description = name
        self._author = ''
        self._version = ''
        self._role = role
        #reports info
        self._info = info
        
        self._running = False
        self._result = None
        self._running_script = None
        self._script_args = None
        self._script_kwargs = None
        self._end_msg = None
        self._end_stack = None
        self._end_args = None
        self.script_dir = script_dir

        self.bench = TestScriptEnvironment(self, self.result_handler, self.logger)

        self.function = self._load()
        self.timeout = timeout

        if log is None:
            self.log = logging.getLogger()
        else:
            self.log = log

        self._deferred = deferred
        if deferred is None:
            self._deferred = defer.Deferred()
        else:
            self._deferred = deferred

    @property
    def name(self):
        """The script name."""
        return self._name

    @property
    def description(self):
        """A description of the script behavior."""
        return self._description

    @property
    def short_name(self):
        """A short name for the script."""
        return self._short_name

    @property
    def author(self):
        """The author of the script."""
        return self._author

    @property
    def version(self):
        """The version of the script."""
        return self._version

    @property
    def role(self):
        """The role of the service executing the script."""
        return self._role
    
    @property
    def passed(self):
        """True if the test has passed."""
        return self.result.passed

    @property
    def failed(self):
        """True if the test has failed."""
        return self.result.failed

    @property
    def running(self):
        """True if the test is running."""
        return self._running

    @property
    def timedout(self):
        """True if the test timedout ."""
        return self._timedout

    @property
    def script_args(self):
        """A tuple with the arguments passed to the script."""
        return self._script_args

    @property
    def script_kwargs(self):
        """A dict of keyword arguments passed to the script."""
        return self._script_kwargs

    @property
    def filename(self):
        """The filename of the script."""
        return os.path.join(self.script_dir, '%s.py' % self.name)

    @property
    def result(self):
        """The test result."""
        return self._result
    
    
    
    #reports info properties
    def _get_customer(self):
        """The customer who is testing."""
        return self._get_info('customer')
    
    def _get_auditor(self):
        """The auditor of the script."""
        return self._get_info('auditor')
    
    def _get_tcu(self):
        """The tcu which is communicating."""
        return self._get_info('tcu')
    
    def _get_server(self):
        """The server of the script."""
        return self._get_info('server')
    
    def _get_reference(self):
        """A description for the script."""
        return self._get_info('reference')
    
    def _set_customer(self, value):
        self._set_info('customer', value)
    
    def _set_auditor(self, value):
        self._set_info('auditor', value)
    
    def _set_tcu(self, value):
        self._set_info('tcu', value)
    
    def _set_server(self, value):
        self._set_info('server', value)
    
    def _set_reference(self, value):
        self._set_info('reference', value)
    
    customer = property(_get_customer, _set_customer)
    auditor = property(_get_auditor, _set_auditor)
    tcu = property(_get_tcu, _set_tcu)
    server = property(_get_server, _set_server)
    reference = property(_get_reference, _set_reference)
    
    def _get_info(self, key):
        return '' if key not in self._info else self._info[key]
    
    def _set_info(self, key, value):
        self._info[key] = value
    
    
    
    def add_result_handler(self, result_handler):
        """Adds a handler for this script results.
        @param result_handler an ITestResultHandler."""
        self.result_handler.add_handler(result_handler)

    def add_logger(self, logger):
        """Adds a logger for log messages emited by the script.
        @param logger an ITestLogger."""
        self.logger.add_logger(logger)

    def start(self, conn, *args, **kwargs):
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
        self._script_args = (conn,) + tuple(args)
        self._script_kwargs = dict(kwargs)

        self._running_script = self.function(conn, *args, **kwargs)
        self.result_handler.start(self)
        return self._running_script

    def next(self):
        """Performs the next step of the script main function.
        @raises StopIteration if the script ends (either succeeding or
        failing)."""
        if self._running_script:
            return self._running_script.next()
        else:
            raise StopIteration()

    def abort(self):
        """Aborts the execution of the script. If the script has already been
        stopped, it does nothing."""
        self._result = TestResult(
            TestResult.FAILED,
            'Script aborted',
        )
        self._stop()

    def got_exception(self):
        """Called by a service that is executing a script when an exception
        ocurred while performing actions requested by the script.

        This function is rarely called, but is necessary when the script
        registers a timer or performs other actions that will be done on
        behalf of the script but not while executing the script main function.
        On those cases, this function can be called to notify the script of an
        error and make it abort with the last raised exception."""
        self._result = TestResult(
            TestResult.FAILED,
            str(sys.exc_info()[1]),
            stack=_find_tb(self.filename)
        )
        self._stop()

    def test_failed(self, msg='failed', *args):
        """Fails the test."""
        self._result = TestResult(
            TestResult.FAILED,
            msg,
            args=args,
            stack=_find_stack(self.filename),
        )
        self._stop()

    def test_passed(self, msg='passed', *args):
        """Passes the test."""
        self._result = TestResult(TestResult.PASSED, msg, args=args)
        self._stop()

    def get_source(self):
        """Returns the source code of the script."""
        f = open(self.filename, 'r')
        try:
            return f.read()
        finally:
            f.close()

    def save_as(self, source, name):
        """Saves the current script using the new source under another name.
        This script name will be changed to the new name."""

        old_name = self.name
        try:
            self._name = name
            if os.path.exists(self.filename):
                raise Exception("File already exists")
            self.update(source)
        except:
            self._name = old_name
            raise

    def update(self, source):
        """Saves the current script using the new source."""

        #FIXME: Validate on sandbox...

        globals = self._get_globals()
        restricted_exec(self.bench, source, 'new', globals)
        if 'script' not in globals:
            raise Exception("Must define a script function")

        f = open(self.filename, 'w')
        try:
            f.write(source)
        finally:
            f.close()

        self.function = self._load()

    def _stop(self):
        """Stops the script by setting running to False and running the next
        step of the script."""
        self._running = False
        try:
            # gi_running == generator is running. If True, _stop has been called
            # by the script itself so we don't need to run the next step.
            if self._running_script and not self._running_script.gi_running:
                self._running_script.next()
        except StopIteration:
            assert self._running_script is None
            pass

    def _get_globals(self):
        """Returns the symbols exported to the script."""
        globals = {
            'Result' : Result,
            'autoreply' : self.autoreplies,
            'bench' : self.bench,
            'assert_equals' : self.bench.assert_equals,
            'assert_false' : self.bench.assert_false,
            'assert_true' : self.bench.assert_true,
            'fail' : self.bench.test_failed,
            'random' : random.random,
            'time' : lambda: int(time.time()),
            'as_int' : as_int,
            'as_str' : as_str,
            'parse_args': lambda x, msg: \
                acpyd.args.insert_args_in_msg(FLATTENERS[self.send_msg], x, msg)
        }

        for item in dir(acp245.pdu):
            if not item.startswith('_'):
                globals[item] = getattr(acp245.pdu, item)
        for item in acpyd.args.__all__:
            if not item.startswith('_'):
                globals[item] = getattr(acpyd.args, item)

        globals['stdmsg'] = ReadMsgDict(acp245.stdmsg.STANDARD_BY_SECTION_HEX)
        return globals

    def _load(self):
        """ Returns a new script instance.
        The loaded script will be wrapped to handle different results and
        provide the script with an execution context (by exporting symbols
        to the script globals).
        """

        # TODO: the _load function could accept a parameter dictionary to setup
        # the script global variables. In this way, it would be possible to
        # execute the script with different values for their global values.

        # load the symbols that will be exported TO the script
        f = open(self.filename, 'r')
        try:
            source = f.read()
        finally:
            f.close()

        globals = self._get_globals()
        restricted_exec(self.bench, source, self.filename, globals)
        source = None

        # get special symbols exported BY the script.

        # script = script entry point function
        script = globals['script']

        self._short_name = globals.get('short_name', self.name)
        self._description = globals.get('description', '')
        self._author = globals.get('author', '')
        self._version = globals.get('version', '')
        # timeout specified on the script overrides default timeout
        if 'timeout' in globals:
            self.timeout = int(globals['timeout'])

        # used to automatically process arguments to the scripts
        # and the script result
        self.send_msg = globals.get('arg_msg')
        self.arguments = globals.get('arguments')
        self.recv_msg = globals.get('resp_msg')
        self.response = globals.get('response')

        return self._get_wrapped_script(script)

    def _get_wrapped_script(self, script):
        """Wraps the given script in a function that handles script results and
        errors."""

        def _wrapped_script(*args, **kwargs):
            timeout_timer = None
            if self.timeout:
                def timeout_handler():
                    self._timedout = True
                    self._result = TestResult(
                        TestResult.FAILED,
                        'test timedout'
                    )
                    self._stop()
                timeout_timer = reactor.callLater(self.timeout, timeout_handler)

            # set when test_failed/test_passed is called
            self._result = None
            # set when timeout is reached
            self._timedout = False
            # set when stop is called
            self._running = True

            yield_timeout_timer = None

            try:
                last_res = None
                try:
                    s = script(*args, **kwargs)

                    # if the script is not a generator its because
                    # it doesn't use the 'yield' keyword, therefore
                    # it just executes once and returns a value immediately.
                    if not isinstance(s, types.GeneratorType):
                        last_res = s
                        raise StopIteration()

                    while self.running:
                        # run until the next 'yield' on the script
                        if yield_timeout_timer:
                            if yield_timeout_timer.active():
                                yield_timeout_timer.cancel()
                            yield_timeout_timer = None
                        last_res = s.next()
                        if last_res is not None:
                            if isinstance(last_res, int):
                                # yielded an int,register a timeout to
                                # execute when the yielded number of seconds
                                # elapse
                                def resume(*args):
                                    try:
                                        self.next()
                                    except:
                                        pass
                                yield_timeout_timer = self.bench.timer(last_res,
                                                                       resume)
                            elif isinstance(last_res, Result):
                                # yielded a result, the script ends with that
                                # value
                                last_res = last_res.value
                                raise StopIteration
                            # else, script keeps running but last value is
                            # stored
                        yield

                    assert self.result is not None, "No result after stop"

                except StopIteration:
                    # Script came to an end before calling _stop()
                    if not self.result:
                        self.log.debug("Script ended: %s" % self.name)
                        self._result = TestResult(
                            TestResult.PASSED,
                            'script ended without failures'
                        )

                        if last_res is not None:
                            flattener = FLATTENERS.get(last_res.__class__, None)
                            if flattener is not None:
                                # automagically parse the result using the
                                # appropiate flattener
                                last_res = acpyd.args.insert_msg_in_args(
                                    flattener, last_res
                                )

                except GeneratorExit:
                    # script generator reference lost, script was aborted
                    # (another script has been selected or no references are
                    # left to the script)
                    pass

                except:
                    self.log.exception("Script failure (%s)", self.name)
                    self._result = TestResult(
                        TestResult.FAILED,
                        str(sys.exc_info()[1]),
                        stack=_find_tb(self.filename),
                    )

                if self.passed:
                    self._passed(last_res)
                elif self.failed:
                    self._failed()
                elif self.timedout:
                    self._timeout()
                else:
                    self._aborted()

                # ended, aborted or failed, in any case, script stopped
                raise StopIteration()
            finally:
                self.log.debug("Script cleanup: %s" % self.name)
                # cancel timers registered by the script
                self.bench.cancel_timers()

                # cancel timeout timer
                if timeout_timer and timeout_timer.active():
                    try:
                        timeout_timer.cancel()
                    except:
                        pass
                timeout_timer = None

                #clear state
                self._running_script = None
        return _wrapped_script

    def _passed(self, result):
        """Called when the script has passed.
        Notifies handlers and callbacks the deferred, if any."""
        self.result_handler.passed(self, self.result.msg, *self.result.args)
        if not self._deferred.called:
            self.log.debug("calling callback: %s" % self.name)
            self._deferred.callback(result)

    def _failed(self):
        """Called when the script has failed.
        Notifies handlers and errbacks the deferred, if any."""
        self.result_handler.failed(self, self.result.stack, self.result.msg,
                                   *self.result.args)
        if not self._deferred.called:
            self.log.debug("calling errback: %s" % self.name)
            self.type, self.value, tb = sys.exc_info()
            if self.type is None:
                self._deferred.errback(failure.Failure(Exception(
                    self.result.msg % self.result.args))
                )
            else:
                self._deferred.errback()

    def _timeout(self):
        """Called when the script has timedout.
        Notifies handlers and timesout the deferred, if any."""
        self.result_handler.failed(self, self.result.stack, self.result.msg,
                                  *self.result.args)
        if not self._deferred.called:
            self.log.debug("calling timeout: %s" % self.name)
            defer.timeout(self._deferred)

    def _aborted(self):
        """Called when the script has been aborted.
        Notifies handlers by calling failed and timesout the deferred, if any."""
        self.result_handler.failed(self, self.result.stack, self.result.msg,
                                  *self.result.args)
        if not self._deferred.called:
            self.log.debug("calling errback (aborted): %s" % self.name)
            self._deferred.errback(failure.Failure(Exception("Script aborted")))

    def __str__(self):
        """Returns a string repr of the test case."""
        return '%s[name=%s]' % (self.__class__.__name__, self.name)

class TestScriptToAutoReplyManager(object):
    """Adapts a TestScript to an IAutoReplyManager.
    It just delegates to the AutoReplyManager instance of the TestScript."""
    implements(IAutoReplyManager)
    def __init__(self, script):
        self._test = script
    def has_autoreply(self, msg):
        """Delegates to AutoReplyManager.has_autoreply"""
        return self._test.autoreplies.has_autoreply(msg)
    def get_autoreply(self, msg):
        """Delegates to AutoReplyManager.get_autoreply"""
        return self._test.autoreplies.get_autoreply(msg)
    def __setitem__(self, name, value):
        """Delegates to AutoReplyManager.__setitem__"""
        self._test.autoreplies[name] = value
    def __getitem__(self, name):
        """Delegates to AutoReplyManager.__getitem__"""
        return self._test.autoreplies[name]
    def __delitem__(self, name):
        """Delegates to AutoReplyManager.__delitem__"""
        del self._test.autoreplies[name]

# Register a ITestScript to IAutoReplyManager adapter.
components.registerAdapter(
    TestScriptToAutoReplyManager,
    ITestScript,
    IAutoReplyManager
)

class ScriptProvider(object):
    """A provider of scripts."""
    implements(ITestScriptProvider)
    def __init__(self,
                 script_dir,
                 default_timeout=0,
                 log=None,
                 prefix=''):
        self.script_dir = script_dir
        self.default_timeout = default_timeout
        self.prefix = prefix

        self.result_handler = MultiResultHandler()
        self.logger = MultiLogger()

        if log is None:
            self.log = logging.getLogger()
        else:
            self.log = log

    def add_result_handler(self, result_handler):
        """Adds a result handler that will be added to every script returned by
        this provider when calling get_script."""
        self.result_handler.add_handler(result_handler)

    def add_logger(self, logger):
        """Adds an script logger that will be added to every script returned by
        this provider when calling get_script."""
        self.logger.add_logger(logger)

    def set_script_dir(self, script_dir):
        """Sets the directory to load scripts from."""
        self.script_dir = script_dir

    def get_scripts(self):
        """Get all the scripts stored by this provider.
        @return a list of scripts."""
        scripts = []
        def load(arg, dirname, fnames):
            for sname in [x[:-3] for x in fnames if x.endswith('.py') and
                          x.startswith(self.prefix)]:
                scripts.append(self._load_script(sname,role=''))
        os.path.walk(self.script_dir, load, None)
        return scripts

    def get_script(self, script_name, role='n/a', timeout=None, deferred=None, info={}):
        """Loads and returns the given script.
        @param role an identification string for the script runner.
        @param timeout a number in seconds that the script should be allowed to run.
        @param deferred a deferred to callback when the script passes, or errback
        when the script fails (either because of a failure, exception or
        timeout).
        @params info informational fields shown in reports, recognized are: customer, auditor, tcu, server, reference (comes from report_config in bench)
        @return the script."""
        script = self._load_script(
            script_name, role=role, timeout=timeout, deferred=deferred, info=info)

        # chain observers
        script.add_logger(self.logger)
        script.add_result_handler(self.result_handler)
        return script

    def _load_script(self, script_name, role='n/a', timeout=None, deferred=None, info={}):
        """Creates a new script.
        This can be overriden to return a different instance of ITestScript."""
        if timeout is None:
            timeout = self.default_timeout

        return TestScript(
            self.script_dir,
            script_name,
            role=role,
            timeout=timeout,
            deferred=deferred,
            log=self.log,
            info=info)
