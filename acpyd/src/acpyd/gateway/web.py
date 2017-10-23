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

import exceptions
import urllib

from twisted.internet import defer, reactor
from twisted.web import http
from nevow import rend, loaders, tags as T, inevow

from acpyd.web import (
    ServiceControlButton,
    ServiceStatusLabel,
    TestbenchLog,
    BasePage,
    ReportConfig
)
from acpyd.protocol import SERVER_ROLE, CLIENT_ROLE
from acpyd.gateway import config
from acpyd.testdb.web import TestdbPage, TestdbGrid, PdfReportDialog

class GatewayBasePage(BasePage):
    docFactory = loaders.xmlfile(templateDir=config.TEMPLATE_DIR,
                                 template='gateway_site.html')


class ServicePage(GatewayBasePage):
    template = 'gateway.html'

    def __init__(self, bench, prefix, authorizator=None):
        super(ServicePage, self).__init__(authorizator=authorizator)
        self.bench = bench
        self.prefix = prefix

        self._render('testbench_log', TestbenchLog, bench)

    def get_commands(self):
        commands = [
            ('%s/start' % self.prefix, 'Start'),
            ('%s/stop' % self.prefix, 'Stop')]
        for script in self.bench.get_scripts('%s/' % self.prefix):
            commands.append((script.name, script.short_name))
        return commands

    def render_command_list(self, request, tag):
        div = T.div()
        for path, desc in self.get_commands():
            div.children.append(T.a(
                href='#',
                onclick='gateway_run_command(\'%s\');' % path)[desc])
            div.children.append(T.br())
        return div

    def renderHTTP(self, ctx):
        request = inevow.IRequest(ctx)

        script = '/'.join(request.prepath[1:])

        if not self.auth_http(request):
            # Try to authentication on credentials sent on query string
            user = request.args.get('user',[None])[0]
            password = request.args.get('password',[None])[0]
            if not self.allowed(user, password):
                request.setResponseCode(http.UNAUTHORIZED)
                return "Authentication required."

        if script == '%s' % self.prefix:
            return rend.FourOhFour()

        http_d = defer.Deferred()
        script_d = self._get_result_callback(http_d)

        self.bench.log.debug("Running gateway command at: %s" % script)

        if script == '%s/start' % self.prefix:
            try:
                self._start_service(request.args)
                script_d.callback(None)
            except Exception, e:
                script_d.errback()
        elif script == '%s/stop' % self.prefix:
            try:
                d = self._stop_service(request.args)
                if d:
                    d.chainDeferred(script_d)
                else:
                    script_d.callback(None)
            except Exception, e:
                script_d.errback()
        else:
            # get the timeout argument
            if 'timeout' in request.args:
                timeout = int(request.args['timeout'][0])
            else:
                timeout = None

            try:
                args = {}
                for key, val in request.args.items():
                    args[key] = val[0]

                if timeout:
                    def timeout_handler():
                        if not http_d.called:
                            http_d.callback(urllib.urlencode((('result',
                                                               'TIMEOUT'),)))
                            self._stop_script()
                    timeout_timer = reactor.callLater(timeout, timeout_handler)
                    def cancel_timer(res):
                        if timeout_timer and not timeout_timer.called:
                            timeout_timer.cancel()
                        return res
                    http_d.addBoth(cancel_timer)
                self._set_script('%s/%s' % (self.prefix, script.split('/')[-1]),
                                 args, script_d, timeout=timeout)
            except exceptions.IOError, e:
                self.bench.log.warn('Script not found: %s (%s)' % (script, e))
                return rend.FourOhFour()
            except Exception, e:
                self.bench.log.exception('Internal server error')
                request.setResponseCode(http.INTERNAL_SERVER_ERROR)
                return "Internal server error"

        return http_d

    def _get_result_callback(self, http_d):
        script_d = defer.Deferred()
        def script_ok(*args):
            #FIXME this stop_script here is ad-hoc
            self._stop_script()
            res = [('result', 'OK')]
            if args:
                for arg in args:
                    if isinstance(arg, tuple) or isinstance(arg, list):
                        res.extend(arg)
                    elif isinstance(arg, dict):
                        res.extend(arg.items())
                    elif arg is not None:
                        res.append(('value',str(arg)))
            if not http_d.called:
                http_d.callback(urllib.urlencode(res))

        def script_error(f, *args):
            #FIXME this stop_script here is ad-hoc
            self._stop_script()
            if f.type == defer.TimeoutError:
                res = (('result', 'TIMEOUT'),)
            elif args:
                res = (('result', 'ERROR'),('error',str(args)))
            else:
                res = (('result', 'ERROR'),('error',str(f.getErrorMessage())))
            if not http_d.called:
                http_d.callback(urllib.urlencode(res))

        def fatal_error(*args):
            self.bench.log.error('Fatal error processing test result %s', args[0])
            res = (('result','ERROR'),('internal',True))
            if not http_d.called:
                http_d.callback(urllib.urlencode(res))

        script_d.addCallbacks(script_ok, script_error).addErrback(fatal_error)
        return script_d

    def childFactory(self, ctx, name):
        return self.__class__(self.bench, authorizator=self.authorizator)

    def _stop_script(self):
        raise NotImplementedError()

class ClientPage(ServicePage):
    def __init__(self, bench, authorizator=None):
        super(ClientPage, self).__init__(bench,
                                         'client',
                                         authorizator=authorizator)
    def _start_service(self, args):
        self.bench.add_client(
            ip=args.get(
                'ip',
                [self.bench.client.ip]
            )[0],
            port=int(args.get(
                'port',
                [self.bench.client.port]
            )[0])
        )
        self.bench.start_client()
    def _stop_service(self, args):
        return self.bench.stop_client()
    def _stop_script(self):
        self.bench.clear_client_script()
    def _set_script(self, script, args, d, timeout=None):
        self.bench.set_client_script(script, args=args, timeout=timeout, deferred=d)

class ServerPage(ServicePage):
    def __init__(self, bench, authorizator=None):
        super(ServerPage, self).__init__(bench,
                                         'server',
                                         authorizator=authorizator)
    def _start_service(self, args):
        self.bench.add_server(
            port=int(args.get(
                'port',
                [self.bench.client.port]
            )[0])
        )
        self.bench.start_server()
    def _stop_service(self, args):
        return self.bench.stop_server()
    def _stop_script(self):
        self.bench.clear_server_script()
    def _set_script(self, script, args, d, timeout=None):
        self.bench.set_server_script(script, args=args, timeout=timeout, deferred=d)


class MainPage(GatewayBasePage):
    template = 'gateway_control.html'

    def __init__(self, bench, authorizator=None):
        super(MainPage, self).__init__(authorizator=authorizator)
        self.bench = bench

        self._render('report_config_editor', ReportConfig, self.bench)
        self._render('testbench_log', TestbenchLog, self.bench)

        self._render_if(
            lambda: self.bench.server_enabled,
            'server_control_button', ServiceControlButton, self.bench.server)
        self._render_if(
            lambda: self.bench.server_enabled,
            'server_status_label', ServiceStatusLabel, self.bench.server)

        self._render('testdb_grid', TestdbGrid)
        
        self._render('pdf_report_dialog', PdfReportDialog, bench)

    def renderHTTP(self, ctx):
        request = inevow.IRequest(ctx)

        script = '/'.join(request.prepath[1:])
        if script == '':
            if len(request.prepath) == 1:
                request.redirect(request.uri + '/')
                return ''
            else:
                return super(MainPage, self).renderHTTP(ctx)

        if not self.auth_http(request):
            # Try to authentication on credentials sent on query string
            user = request.args.get('user',[None])[0]
            password = request.args.get('password',[None])[0]
            if not self.allowed(user, password):
                request.setResponseCode(http.UNAUTHORIZED)
                return "Authentication required."

        if script == 'start_test':
            name_arg = request.args.get('name')
            if name_arg:
                name = name_arg[0]
            else:
                name = None
            self.bench.start_test(name)
            return 'result=OK'
        elif script == 'stop_test':
            self.bench.stop_test()
            return 'result=OK'
        else:
            self.bench.log.warn('Script not found: %s' % script)
            return rend.FourOhFour()

    def childFactory(self, ctx, name):
        if name == 'client':
            if self.bench.client_enabled:
                return ClientPage(self.bench, authorizator=self.authorizator)
            else:
                return rend.FourOhFour()
        elif name == 'server':
            if self.bench.server_enabled:
                return ServerPage(self.bench, authorizator=self.authorizator)
            else:
                return rend.FourOhFour()
        elif name == 'testdb':
            return TestdbPage(self.bench, authorizator=self.authorizator)
        else:
            return MainPage(self.bench, authorizator=self.authorizator)
