from zope.interface import implements

from acpyd.interfaces import ITestLogStore
from acpyd.testobservers import (
        TestbenchLogObserver,
        TestbenchLogResultHandler,
        TestbenchLogLogger,
        TEST_STARTED,
        TEST_FAILED
)

class TestDbLogStore(object):
    """A test log store that stores events on a database."""
    implements(ITestLogStore)

    def __init__(self, testdb):
        self.testdb = testdb
        self.test = None

    def start_test(self, script):
        """Called when a test is started. Creates a new test run register on the
        database to store log events under."""
        self.test = self.testdb.new_test(script.name, customer=script.customer, auditor=script.auditor, tcu=script.tcu, server=script.server, reference=script.reference)

    def close_test(self):
        """Called when a test is ended. Closes the current test run register."""
        self.test = None

    def log(self, role, event, text, host=None, peer=None, msg=None, error=None,
            level=None, data=None):
        """Adds a new test log line register on the DB under the current test
        run register."""
        if self.test:
            self.test.newTestLogLine(level or 'info', role, event, text,
                                     data=data, host=host, peer=peer)

class TestDbObserver(
    TestbenchLogObserver,
    TestbenchLogResultHandler,
    TestbenchLogLogger
):
    """An observer/logger/handler that stores events on a TestDbLogStore."""
    # TODO: too/badly coupled, I'm afraid this design represents a
    # misunderstanding of twisted components system. Refactoring is recommended.

    # ITestResultHandler implementation
    def start(self, script):
        """Called when a test script starts."""
        self.log.start_test(script)
        TestbenchLogResultHandler.start(self, script)

    def failed(self, script, stack, msg, *args):
        """Called when a test script fails."""
        TestbenchLogResultHandler.failed(self, script, stack, msg, *args)
        self.log.close_test()

    def passed(self, script, msg, *args):
        """Called when a test script passes."""
        TestbenchLogResultHandler.passed(self, script, msg, *args)
        self.log.close_test()
