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

import twisted.trial.unittest as unittest
import os
import shutil

from acp245 import pdu
from acpyd.testdb import TestDb, TestRun
from axiom import attributes
from epsilon import extime
import time

import acpyd.test.compat

class TestDbTest(unittest.TestCase):
    def test_upgrade_1_to_2(self):
        """
        import acpyd.test as test_module
        filename = os.path.join(
            os.path.split(test_module.__file__)[0],
            'testdb.console.1'
        )

        if os.path.isdir('test_db'):
            shutil.rmtree('test_db')
        shutil.copytree(filename, 'test_db')

        db = TestDb('test_db')
        return db.store.whenFullyUpgraded()
        """
    def test_testdb(self):
        """
        if os.path.isdir('test_db'):
            shutil.rmtree('test_db')
        db = TestDb('test_db')
        test = db.new_test(u'foobar')
        self.assertEquals(u'foobar', test.name)

        test_2 = db.get_test(1)
        self.assert_(test_2 is not None)
        self.assertEquals(test.date, test_2.date)
        self.assertEquals(test.name, test_2.name)

        test.newTestLogLine('debug', 'my_role', 'my_event', 'my text',
                            pdu.ProvUpd().as_bytes())
        test.newTestLogLine('info', 'your_role', 'your_event', 'your text',
                            pdu.ProvUpd().as_bytes())
        records = test.get_logs()
        self.assertEquals(2, len(records))

        self.assertEquals(u'my_event', records[0].event)
        self.assertEquals(u'my_role', records[0].role)
        self.assertEquals(u'debug', records[0].level)
        self.assertEquals(records[0].pdu_dict()['type'], pdu.ProvUpd.type)

        self.assertEquals(u'your_event', records[1].event)
        self.assertEquals(u'your_role', records[1].role)
        self.assertEquals(u'info', records[1].level)
        self.assertEquals(records[1].pdu_dict()['type'], pdu.ProvUpd.type)

        q = db.query(TestRun, TestRun.name.like(u'%ob%'))
        self.assertEquals(1, q.count())
        self.assertEquals(1, len(list(q)))
        end_of_day = extime.Time.fromStructTime(
            time.strptime(
                time.strftime('%m/%d/%y 23:59:59'),
                '%m/%d/%y %H:%M:%S'
            )
        )
        q = db.query(TestRun,
                     attributes.AND(
                         TestRun.date >= end_of_day.oneDay(),
                         TestRun.date <= end_of_day))
        self.assertEquals(1, q.count())
        self.assertEquals(1, len(list(q)))
        """
