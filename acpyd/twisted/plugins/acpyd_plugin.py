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
from zope.interface import implements
from twisted.python import usage
from twisted.plugin import IPlugin
from twisted.application.service import IServiceMaker
import os

SKEL_DIR = 'vhost-skel'
HOSTS_DATA_DIR = 'vhosts'

class Options(usage.Options):
    optParameters = [
        ['config', 'c', '/etc/acpyd/acpyd.conf',
         "Configuration file"],
        ['log-dir', 'l', '/var/log/acpyd/',
         "Logging directory"],
        ["certificate", "", "server.pem", "SSL certificate to use for HTTPS. "],
        ["privkey", "", "server.pem", "SSL certificate to use for HTTPS."],
    ]

class AcpydServiceMaker(object):
    implements(IServiceMaker, IPlugin)

    tapname = "acpyd"
    description = "Edantech ACP 245 gateway"
    options = Options

    def read_config(self, cfg_fname):
        import twisted.plugins.acpyd_plugin
        from acpyd.configobj import ConfigObj, flatten_errors
        from acpyd.validate import Validator
        if os.path.isfile(cfg_fname):
            cspec_fname = os.path.join(
                os.path.split(twisted.plugins.acpyd_plugin.__file__)[0],
                'acpyd_config.cspec'
            )
            configspec = ConfigObj(cspec_fname,
                                   encoding='UTF8',
                                   list_values=False,
                                   _inspec=True)
            validator = Validator()
            config = ConfigObj(cfg_fname, configspec=configspec)
            results = config.validate(validator)
            if results != True:
                errors = ''
                for (section_list, key, _) in flatten_errors(config, results):
                    if key is not None:
                        errors += '\nThe "%s.%s" option is invalid.' \
                                % (','.join(section_list), key)
                    else:
                        errors += '\nThe "%s" section was missing' \
                                % ', '.join(section_list)
                raise Exception(errors)
            else:
                return config
        else:
            raise Exception('Config file not found: %s' % cfg_fname)

    def makeService(self, options):
        from acpyd.bench import VirtualHostsTestBenchService
        from twisted.application import service, internet
        from twisted.internet import ssl
        from nevow import appserver

        config = self.read_config(options['config'])
        acpyd_service = VirtualHostsTestBenchService(
            config, options['log-dir'], SKEL_DIR, HOSTS_DATA_DIR)

        internet.TCPServer(
            int(config['http-port']),
            appserver.NevowSite(acpyd_service.main_page)
        ).setServiceParent(acpyd_service)

        if 'https-port' in config:
            internet.SSLServer(
                int(config['https-port']),
                appserver.NevowSite(acpyd_service.main_page),
                ssl.DefaultOpenSSLContextFactory(
                        config['privkey'],
                        config['certificate'])
            ).setServiceParent(acpyd_service)

        return acpyd_service
serviceMaker = AcpydServiceMaker()
