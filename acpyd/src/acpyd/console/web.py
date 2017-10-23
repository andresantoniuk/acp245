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
from twisted.python import htmlizer, components
from nevow import page, rend, athena, loaders, tags as T, inevow

from acpyd.web import (
    LiveSelect,
    ServiceControlButton,
    ServiceStatusLabel,
    TestbenchLog,
    BasePage,
    BaseElement,
    ReportConfig
)
from acpyd.interfaces import ITestObserver
from acpyd.console import config
from acpyd.protocol import SERVER_ROLE, CLIENT_ROLE

from acpyd.testdb.web import TestdbPage, TestdbGrid, PdfReportDialog

def _python_highlighter(text):
    """Returns an HTML syntax-highlighted version of the given python script."""
    from StringIO import StringIO
    out = StringIO()
    htmlizer.filter(StringIO(text), out)
    return out.getvalue()

class ServiceScriptSelector(LiveSelect):
    """A select widgets that when changed changes the associated service
    script."""
    def __init__(self, bench):
        """Creates a new select widget.
        @param bench the scripted service
        """
        LiveSelect.__init__(self)
        self.bench = bench

    def _get_items(self):
        """Returns the names of the scripts handled by the widget scripted
        service."""
        prefix = '%s_' % self.bench.role

        def strip_name(name):
            return name[len(prefix):]

        scripts = [strip_name(x.name) for x in self._get_scripts() if
                   x.name.startswith(prefix)]
        scripts.sort()
        return ([(name, name) for name in scripts],
                strip_name(self._current_script_name()))

    def _current_script_name(self):
        """Returns the current script name. Must be overriden by subclasses."""
        raise NotImplementedError

    def _get_scripts(self):
        """Returns the service scripts. Must be overriden by subclasses."""
        raise NotImplementedError

    def _selected(self, value):
        """Called from the browser when the script is selected."""
        self.bench.set_script('%s_%s' % (self.bench.role, value))

    def _current_script_name(self):
        """Returns the current script name."""
        if self.bench.get_script() is not None:
            return self.bench.get_script().name
        else:
            return ''

    def _get_scripts(self):
        """Returns the service scripts."""
        return self.bench.get_scripts()

class ScriptEditor(BaseElement):
    """A widget that allows a user to edit an script and save it as the script
    used by a scripted service."""
    jsClass = u"acpyd.ScriptEditor"
    docFactory = loaders.xmlfile(templateDir=config.TEMPLATE_DIR,
                                 template='editor.html',
                                 pattern='ScriptEditorPattern')
    def __init__(self, bench):
        """Creates the editor.
        @param bench the scripted service.
        """
        BaseElement.__init__(self)
        self.bench = bench

        # register appropiate renderers.
        self._render('status_label', ServiceStatusLabel, bench)
        self._render('script_selector', ServiceScriptSelector, bench)
        self._render('control_button', ServiceControlButton, bench,
                     text_down='stop',
                     text_up='start')

    def view_script(self):
        """Called when the script needs to be shown on the browser.
        Returns the script source code highlighted as HTML, or as raw text,
        depending on acpyd.console.config.PYTHON_HIGHLIGHT."""
        script = self.bench.get_script()
        if config.PYTHON_HIGHLIGHT:
            text = _python_highlighter(script.get_source())
        else:
            text = script.get_source()
        return unicode(text)
    athena.expose(view_script)

    def view_script_raw(self):
        return unicode(self.bench.get_script().get_source())
    athena.expose(view_script_raw)

    def save_script(self, source):
        if not self.bench.write_script:
            raise Exception('not authorized')
        # dos2unixize...
        source = source.replace('\r\n','\n')
        self.bench.get_script().update(source)
    athena.expose(save_script)

    def save_as_script(self, source, new_name):
        if not self.bench.write_script:
            raise Exception('not authorized')
        # dos2unixize...
        source = source.replace('\r\n','\n')
        name = '%s_%s' % (self.bench.role, new_name.strip())
        self.bench.get_script().save_as(source, name)
        self.bench.set_script(name)
    athena.expose(save_as_script)

    def if_bench_write_script(self, request, tag):
        if self.bench.write_script:
            return tag
        else:
            return ''
    page.renderer(if_bench_write_script)

    def role(self, request, tag):
        return self.bench.role
    page.renderer(role)

    
class ClientPortSelector(LiveSelect):
    def __init__(self, bench):
        self.bench = bench

    def _get_items(self):
        return ([(unicode(x), unicode(x)) for x in self.bench.client.allowed_ports],
                unicode(self.bench.client.port))

    def _selected(self, value):
        port = int(value)
        if port in self.bench.client.allowed_ports:
            self.bench.client.port = port

class ClientIPSelector(LiveSelect):
    def __init__(self, bench):
        self.bench = bench

    def _get_items(self):
        return ([(unicode(x), unicode(x)) for x in self.bench.client.allowed_ips],
                unicode(self.bench.client.ip))

    def _selected(self, value):
        if value in self.bench.client.allowed_ips:
            self.bench.client.ip = value

class ServerPortSelector(LiveSelect):
    def __init__(self, bench):
        self.bench = bench

    def _get_items(self):
        return ([(unicode(x), unicode(x)) for x in self.bench.server.allowed_ports],
                unicode(self.bench.server.port))

    def _selected(self, value):
        port = int(value)
        if port in self.bench.server.allowed_ports:
            self.bench.server.port = port

class ServicePortSelector(LiveSelect):
    def __init__(self, bench):
        self.bench = bench

    def _get_items(self):
        return ([(unicode(x), unicode(x)) for x in self.bench.allowed_ports],
                unicode(self.bench.port))

    def _selected(self, value):
        port = int(value)
        if port in self.bench.allowed_ports:
            self.bench.port = port

class ServiceIpSelector(LiveSelect):
    def __init__(self, bench):
        self.bench = bench

    def _get_items(self):
        return ([(unicode(x), unicode(x)) for x in self.bench.allowed_ips],
                unicode(self.bench.ip))

    def _selected(self, value):
        if value in self.bench.allowed_ips:
            self.bench.ip = value

class ClientConfigForm(BaseElement):
    docFactory = loaders.xmlfile(templateDir=config.TEMPLATE_DIR,
                                 template='client_config_form.html')

    def __init__(self, bench):
        BaseElement.__init__(self)
        self.bench = bench
        self._render('port_selector', ServicePortSelector, bench)
        self._render('ip_selector', ServiceIpSelector, bench)

class ServerConfigForm(BaseElement):
    docFactory = loaders.xmlfile(templateDir=config.TEMPLATE_DIR,
                                 template='server_config_form.html')

    def __init__(self, bench):
        BaseElement.__init__(self)
        self.bench = bench
        self._render('port_selector', ServicePortSelector, bench)

class MainPage(BasePage):
    docFactory = loaders.xmlfile(templateDir=config.TEMPLATE_DIR,
                                 template='console_site.html')
    template = 'console.html'

    def __init__(self, bench, authorizator=None):
        BasePage.__init__(self, authorizator=authorizator)
        self.bench = bench
        self.authorizator = authorizator

        self._render('report_config_editor', ReportConfig, bench)

        self._render('testbench_log', TestbenchLog, bench)

        self._render_if(
            lambda: bench.server_enabled,
            'server_script_editor', ScriptEditor, bench.server)
        self._render_if(
            lambda: bench.server_enabled,
            'server_config', ServerConfigForm, bench.server)

        self._render_if(
            lambda: bench.client_enabled,
            'client_script_editor', ScriptEditor, bench.client)
        self._render_if(
            lambda: bench.client_enabled,
            'client_config', ClientConfigForm, bench.client)

        self._render('testdb_grid', TestdbGrid)

        self._render('pdf_report_dialog', PdfReportDialog, bench)

    def renderHTTP(self, ctx):
        request = inevow.IRequest(ctx)

        script = '/'.join(request.prepath[1:])
        if script == '':
            return BasePage.renderHTTP(self, ctx)

    def childFactory(self, ctx, name):
        if name == 'testdb':
            return TestdbPage(self.bench, authorizator=self.authorizator)
        else:
            return MainPage(self.bench, authorizator=self.authorizator)
