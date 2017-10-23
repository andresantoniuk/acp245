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
"""acpyd test classes."""
import os
import logging
logging.basicConfig(
    level=logging.DEBUG,
    # COMMENT THIS LINE TO SEE ERROR MESSAGES
    filename='/dev/null' if not os.environ.get('VERBOSE',0) else None,
    format='%(asctime)s %(levelname)s %(message)s',
)
