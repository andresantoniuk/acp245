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

from acpyd.flatten import FLATTENERS
from acpyd.args import insert_msg_in_args, insert_args_in_msg

import acpyd.test.compat

class FlattenTest(unittest.TestCase):
    def test_flatteners(self):
        for msg_type, fields in FLATTENERS.items():
            args = insert_msg_in_args(fields, msg_type())
            insert_args_in_msg(fields, args, msg_type())
