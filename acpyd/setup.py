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
from distutils.core import setup

setup(
    name = "acpyd",
    version = "1.7.0",
    description = "Edantech ACP245 daemon",
    author = "Santiago Aguiar",
    author_email = "santiago.aguiar@edantech.com",
    url = "http://www.edantech.com/",
    packages=[
        'acpyd',
        'acpyd.test',
        'acpyd.testdb',
        'acpyd.console',
        'acpyd.gateway',
        'acpyd.RestrictedPython',
    ],
    package_dir={'acpyd': 'src/acpyd'},
    package_data={'acpyd.testdb': ['testdb.console.1/*']},

    classifiers = [
        'License :: (c) Edantech. All rights reserved.'
    ]
)
