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
"""
acpyd basic config.
This module include config parameters that are required statically by some parts
of the code, and therefore can not be included on the configuration object (ie
nevow code).
There's probably a way to avoid this config module and use the config object
directly.
"""

TEMPLATE_DIR = 'templates'
STATIC_DIR = 'static'
ATHENA_JS_DIR = 'js'
SCRIPT_DIR = 'scripts'
