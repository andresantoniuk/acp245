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
"""ACP245 Test Bench services."""

import logging
import logging.handlers
import os
import shutil

from nevow import vhost, rend
from twisted.application import service

from acpyd.service import Authorizator, TestBenchService
from acpyd.frontend import FrontendPage
from acpyd.avlreporter import AvlReporter
from acpyd.gateway.service import GatewayService
from acpyd.gateway.web import MainPage as GatewayMainPage
from acpyd.console.service import ConsoleService
from acpyd.console.web import MainPage as ConsoleMainPage

class VirtualHostsTestBenchService(service.MultiService):
    """Service to host multiple virtual ACP245 server instances, distinguished
    by domain name.

    Each virtual server will have it's own domain name, log file, and data
    directory for scripts and test databases.

    Configuration format is defined on twisted/plugins/acpyd_config.cspec.
    """
    def __init__(self, config, log_dir, skel_dir, host_data_dir):
        """Creates a new virtual servers service.
        * config is a configuration dictionary, parsed from the acpyd
        configuration file (read the file to learn which parameters are
        available)
        * log_dir is the directory where to store log files.
        * skel_dir is the directory from where the common scripts and per virtual
        * server directory structure will be copied.
        * host_data_dir is the directory where to store per virtual server
        directories. The skel_dir will be copied to this directory under a
        directory whose name is the virtual server domain name.
        """
        service.MultiService.__init__(self)
        self.main_page = vhost.NameVirtualHost(default=rend.FourOhFour())
        self.config = config
        self.log_dir = log_dir
        self.skel_dir = skel_dir
        self.host_data_dir = host_data_dir

    def startService(self):
        """Starts the service.
        Configures logging and checks/creates the directories for each virtual
        server.
        """
        # configure root logger
        log = logging.getLogger()
        log_console_format = logging.Formatter(
            '%(asctime)s %(levelname)s %(message)s')
        log_console_handler = logging.StreamHandler()
        # TODO: configure it from the config file
        log_console_handler.setLevel(logging.DEBUG)
        log_console_handler.setFormatter(log_console_format)
        log.addHandler(log_console_handler)

        if self.config['use-vhosts']:
            for host, domain_cfg in self.config['domains'].iteritems():
                if host == 'main':
                    # ignore main domain, used when use-vhosts == False
                    continue
                frontend, bench = self.__make_testbench(
                    host,
                    self.log_dir,
                    os.path.join(self.host_data_dir, host),
                    domain_cfg)
                self.main_page.addHost(host, frontend)
                bench.setServiceParent(self)
        else:
            page, bench = self.__make_testbench(
                'main',
                self.log_dir,
                'main',
                self.config['domains']['main'])
            bench.setServiceParent(self)
            self.main_page.default = page
        service.MultiService.startService(self)

    def __make_testbench(self, host, log_dir, run_dir, cfg):
        """Creates a new testbench service for a virtual server.
        * host is the domain name of the virtual server.
        * log_dir is the directory where to store the log file.
        * run_dir is the directory used to store the virtual server data.
        * cfg is the configuration object.
        """
        if cfg.get('user-pass'):
            authorizator = Authorizator()
            for user_pass in cfg['user-pass']:
                if ':' not in user_pass:
                    raise Exception("invalid user/pass")
                authorizator.add_user(*user_pass.split(':'))
        else:
            authorizator = None

        bench = TestBenchService()

        run_dir = os.path.join(self.host_data_dir, host)
        if not os.path.isdir(run_dir):
            shutil.copytree(self.skel_dir, run_dir)



        frontend = FrontendPage(
            self.__make_logger(log_dir, host, 'frontend', cfg),
            authorizator=authorizator)

        service_cfg = cfg['gateway']
        if service_cfg['enabled']:
            rep_dir=os.path.join(run_dir, 'gw_reports')
            if not os.path.isdir(rep_dir):
                os.mkdir(rep_dir)
            gateway = GatewayService(
                host,
                self.__make_logger(log_dir, host, 'gateway', service_cfg),
                script_dir='gateway_scripts',
                testdb_name=os.path.join(run_dir, 'testdb.gateway'),
                authorizator=authorizator,
                config=service_cfg,
                reports_dir=rep_dir
            )
            gateway.setServiceParent(bench)

            frontend.add_child('gateway', gateway.get_web_ui())

        service_cfg = cfg['console']
        if service_cfg['enabled']:
            rep_dir=os.path.join(run_dir, 'co_reports')
            if not os.path.isdir(rep_dir):
                os.mkdir(rep_dir)
            console = ConsoleService(
                host,
                self.__make_logger(log_dir, host, 'gateway', service_cfg),
                script_dir=os.path.join(run_dir, 'console_scripts'),
                testdb_name=os.path.join(run_dir, 'testdb.console'),
                authorizator=authorizator,
                config=service_cfg,
                reports_dir=rep_dir
            )
            console.setServiceParent(bench)
            if console.client_enabled:
                console.set_client_script('client_1_connect_and_wait')
            if console.server_enabled:
                console.set_server_script('server_1_connect_and_wait')

            frontend.add_child('console', console.get_web_ui())

            if service_cfg['avld']['enabled'] and console.server_enabled:
                reporter = AvlReporter(service_cfg['avld'])
                console.server.add_observer(reporter)
                console.server.add_result_handler(reporter)

        return frontend, bench

    def __make_logger(self, log_dir, host, service_name, cfg):
        """Creates a new logger for a virtual server.
        * log_dir is the directory where to store the log file.
        * host is the domain name of the virtual server.
        * service_name is the name of the acypyd service to log (console, gateway,
        etc.)
        * cfg is a dict that must include the following keys:
            - log-level: the log level.
        """
        log = logging.getLogger('%s_%s' % (host, service_name))
        if cfg['log-level'] == 'debug':
            log.setLevel(logging.DEBUG)
        else:
            log.setLevel(logging.INFO)
        log_handler = logging.handlers.RotatingFileHandler(
            os.path.join(log_dir, host + '.log'),
            maxBytes=5000000,
            backupCount=10
        )
        log_format = logging.Formatter('%(asctime)s %(levelname)s %(message)s')
        log_handler.setFormatter(log_format)
        log.addHandler(log_handler)

        log_console_format = logging.Formatter(
            '%(asctime)s %(levelname)s ' + host + ' %(message)s')
        log_console_handler = logging.StreamHandler()
        log_console_handler.setLevel(logging.ERROR)
        log_console_handler.setFormatter(log_console_format)
        log.addHandler(log_console_handler)
        return log
