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
"""Test database entity definitions."""

import cPickle

import acp245.pdu

from axiom.attributes import text, bytes, timestamp, reference, integer, AND
from axiom.item import Item
from axiom.store import Store
from axiom.upgrade import registerUpgrader
from epsilon import extime

from acpyd.testobservers import TEST_PASSED


def toUnicode(toConvert):
        if isinstance(toConvert, str):
                return unicode(toConvert)
        elif isinstance(toConvert, dict):#convert recursively
                return dict([(key, toUnicode(val)) for key,val in toConvert.iteritems()])
        elif isinstance(toConvert, list):
                return [toUnicode(val) for val in toConvert]
        return toConvert #unsupported, return unchanged


class NoSuchObjectError(Exception):
    pass

class pickle(bytes):
    def infilter(self, pyval, oself, store):
        return super(pickle, self).infilter(cPickle.dumps(pyval, cPickle.HIGHEST_PROTOCOL), oself, store)

    def outfilter(self, dbval, oself):
        return cPickle.loads(super(pickle, self).outfilter(dbval, oself))
# FIXME: axion does not support data types outside attributes...
import axiom.attributes
setattr(axiom.attributes, 'pickle', pickle)

class TestLogLine(Item):
    schemaVersion = 3

    test = reference()
    date = timestamp()
    level = text()
    role = text()
    event = text()
    msg = text()
    data = bytes()
    # kept for backward compat...
    msg_dict = pickle()

    host_name = text()
    host_port = integer(default=0)
    peer_name = text()
    peer_port = integer(default=0)

    def pdu(self):
        if self.data is not None:
             return acp245.pdu.msg_read(self.data)[0]
        return None

    def pdu_dict(self):
        pdu = self.pdu()
        if pdu is not None:
            return pdu.as_dict()
        elif self.msg_dict is not None:
            return self.msg_dict
        else:
            return None

def test_log_line_1_to_2(old):
    new = old.upgradeVersion('acpyd_testdb_testlogline', 1, 2)
    new.data = None
    new.msg_dict = old.msg_dict
    new.test = old.test
    new.date = old.date
    new.level = old.level
    new.role = old.role
    new.event = old.event
    new.msg = old.msg
    return new
registerUpgrader(test_log_line_1_to_2, 'acpyd_testdb_testlogline', 1, 2)

def test_log_line_2_to_3(old):
    new = old.upgradeVersion('acpyd_testdb_testlogline', 2, 3)
    new.data = None
    new.msg_dict = old.msg_dict
    new.test = old.test
    new.date = old.date
    new.level = old.level
    new.role = old.role
    new.event = old.event
    new.msg = old.msg
    #new info fields
    new.customer = u''
    new.auditor = u''
    new.server = u''
    new.tcu = u''
    new.reference = u''
    return new
registerUpgrader(test_log_line_2_to_3, 'acpyd_testdb_testlogline', 2, 3)

class TestRun(Item):
    schemaVersion = 3

    name = text()
    date = timestamp()
    #info fields
    customer = text()
    auditor = text()
    tcu = text()
    server = text()
    reference = text()
    
    bench_version = integer(default=2)

    def __init__(self, *args, **kwargs):
        Item.__init__(self, *args, **kwargs)

    def newTestLogLine(self,
                       level,
                       role,
                       event,
                       text,
                       data=None,
                       host=None,
                       peer=None):
        if host:
            host_name, host_port = host
        else:
            host_name, host_port = None, None
        if peer:
            peer_name, peer_port = peer
        else:
            peer_name, peer_port = None, None
        return TestLogLine(
            store=self.store,
            test=self,
            date=extime.Time(),
            level=unicode(level),
            role=unicode(role),
            event=unicode(event),
            msg=unicode(text),
            data=data,
            host_name=unicode(host_name),
            host_port=host_port,
            peer_name=unicode(peer_name),
            peer_port=peer_port,
        )

    def get_logs(self, start=None, limit=None):
        return list(self.store.query(TestLogLine, TestLogLine.test == self, offset=start, limit=limit))

    def passed(self):
        return bool(list(
            self.store.query(TestLogLine,
                             AND(
                                 TestLogLine.test == self,
                                 TestLogLine.event == TEST_PASSED,
                             ),
                             limit=1)))

    def get_logs_count(self):
        return self.store.count(TestLogLine, TestLogLine.test == self)


def test_run_1_to_2(old):
    new = old.upgradeVersion('acpyd_testdb_testrun', 1, 2)
    new.name = old.name
    new.date = old.date
    new.bench_version = 1
    return new
registerUpgrader(test_run_1_to_2, 'acpyd_testdb_testrun', 1, 2)

def test_run_2_to_3(old):
    new = old.upgradeVersion('acpyd_testdb_testrun', 2, 3)
    new.name = old.name
    new.date = old.date
    new.bench_version = 1
    #new info fields
    new.customer = u''
    new.auditor = u''
    new.server = u''
    new.tcu = u''
    new.reference = u''
    return new
registerUpgrader(test_run_2_to_3, 'acpyd_testdb_testrun', 2, 3)

from twisted.application.service import IService
class TestDb(object):
    def __init__(self, dbname=u'test_logs'):
        self.store = Store(dbname)
        IService(self.store).startService()
        self.last_test = None

    def new_test(self, name=None, customer=None,
                 auditor=None, tcu=None, server=None,
                 reference=None):
        self.last_test = TestRun(
            store=self.store,
            name=unicode(name),
            date=extime.Time(),
            customer=unicode(customer),
            auditor=unicode(auditor),
            tcu=unicode(tcu),
            server=unicode(server),
            reference=unicode(reference)
        )
        return self.last_test

    def remove_test(self, id):
        self.get_test(id).deleteFromStore()

    def query(self, *args, **kwargs):
        return self.store.query(*args, **kwargs)

    def count(self, *args, **kwargs):
        return self.store.count(*args, **kwargs)

    def get_test(self, id):
        l = list(self.store.query(TestRun, TestRun.storeID==id))
        if l:
            return l[0]
        else:
            raise NoSuchObjectError('TestRun[id=%s]' % id)

    def get_tests_count(self):
        return self.store.count(TestRun)

    def get_tests(self, start=None, limit=None):
        return list(self.store.query(TestRun, offset=start, limit=limit))
    
