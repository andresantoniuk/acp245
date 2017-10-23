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
from zope.interface import Interface

class IPDFTestReport(Interface):
    def get_pdf(test_ids, output=None, progressbar_callback=None):
        pass

class IKMLTestReport(Interface):
    def get_kml(test_id):
        pass
