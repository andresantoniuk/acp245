from zope.interface import implements

from acpyd.interfaces import ITestLogStore
from acpyd.testobservers import (
        TestbenchLogObserver,
        TestbenchLogResultHandler,
        TestbenchLogLogger
)

class TestDbLogStore(object):
    implements(ITestLogStore)

    def __init__(self, testdb, bench):
        self.testdb = testdb
        self.bench = bench

    def log(self, role, event, text, host=None, peer=None, msg=None, error=None,
            level=None, data=None):
        if self.bench.current_test:
            self.bench.current_test.newTestLogLine(
                level or 'info', role, event, text,
                data=data, host=host, peer=peer
            )

class TestDbObserver(
    TestbenchLogObserver,
    TestbenchLogResultHandler,
    TestbenchLogLogger
):
    pass
