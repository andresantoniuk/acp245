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
"""Nevow web extensions and widgets for acpyd interfaces."""

import exceptions
import datetime
import time

from zope.interface import implements

from twisted.web import static, http
from twisted.python import components, reflect
from twisted.internet import reactor
from nevow import page, athena, loaders, tags as T, inevow

from acpyd import config
from acpyd.interfaces import (
        ITestObserver,
        ITestLogger,
        ITestResultHandler,
        ITestLogStore,
        BaseTestObserver,
        Observer,
        Observable
)
from acpyd.testobservers import (
        TestbenchLogObserver,
        TestbenchLogLogger,
        TestbenchLogResultHandler
)

class BaseElement(athena.LiveElement):
    """A base element class with helper methods to render subelements.
    Subclasses should define subelements in the constructor by calling
    _render and _render_if.
    """
    def _render_if(self, condition, name, cls, *args, **kwargs):
        """Defines a subelement renderer that is only renderer if condition
        evaluates to True.
        @param condition a bool or a callable that should evaluate to a bool
        @param name the name of the renderer
        @param cls the class of the widget to render
        @param *args arguments for the widget.
        @param **kwargs kwargs for the widget.
        """
        if condition and (not callable(condition) or condition()):
            self._render(name, cls, *args, **kwargs)
        else:
            def renderer(req, data):
                return ''
            setattr(self, name, page.renderer(renderer))

    def _render(self, name, cls, *args, **kwargs):
        """Defines a subelement renderer.
        @param name the name of the renderer
        @param cls the class of the widget to render
        @param *args arguments for the widget.
        @param **kwargs kwargs for the widget.
        """
        def renderer(req, data):
            obj = cls(*args, **kwargs)
            if isinstance(obj, athena.LiveElement):
                obj.setFragmentParent(self)
            return obj
        setattr(self, name, page.renderer(renderer))

class BasePage(athena.LivePage):
    """A base page.

    Every page is based on a base template, called 'site.html' that provides a
    general skeleton for acpyd pages. The base template should call a 'content'
    renderer that will render a template defined by subclasses of this class. In
    this way, all the pages share the same skeleton but can customize their
    content by providing a specific template.

    It also defines mappings to CSS, JS and images, which should be stored on
    config.STATIC_DIR.
    """
    docFactory = loaders.xmlfile(templateDir=config.TEMPLATE_DIR, template='site.html')
    child_css  = static.File('%s/css' % config.STATIC_DIR)
    child_images = static.File('%s/images' % config.STATIC_DIR)
    child_js = static.File('%s/js' % config.STATIC_DIR)

    def __init__(self, authorizator=None):
        athena.LivePage.__init__(self)
        self.authorizator = authorizator

    def _render_if(self, condition, name, cls, *args, **kwargs):
        """Defines a subelement renderer that is only renderer if condition
        evaluates to True.
        @param condition a bool or a callable that should evaluate to a bool
        @param name the name of the renderer
        @param cls the class of the widget to render
        @param *args arguments for the widget.
        @param **kwargs kwargs for the widget.
        """
        if condition and (not callable(condition) or condition()):
            self._render(name, cls, *args, **kwargs)
        else:
            def renderer(self, ctx, data):
                return ''
            setattr(self, 'render_%s' % name, renderer)

    def _render(self, name, cls, *args, **kwargs):
        """Defines a subelement renderer.

        Rendered objects will be examined for a 'disconnected' method, and if
        found, the method will be called when this page is disconnected from the
        client. This method can be used to cleanup.

        @param name the name of the renderer
        @param cls the class of the widget to render
        @param *args arguments for the widget.
        @param **kwargs kwargs for the widget.
        """
        def renderer(self, ctx, data):
            obj = cls(*args, **kwargs)
            if isinstance(obj, athena.LiveElement):
                obj.setFragmentParent(self)
                # if the rendered element has a disconnected handler, call it
                # when the main page is disconnected
                if getattr(obj, 'disconnected', None) is not None:
                    d = self.notifyOnDisconnect()
                    def notify(*args):
                        obj.disconnected()
                    d.addCallback(notify)
                    d.addErrback(notify)
            return ctx.tag[obj]
        setattr(self, 'render_%s' % name, renderer)

    def render_content(self, context, data):
        """Renders the page content.
        Subclasses must provide a 'template' property which is the name of an
        HTML tempate under config.TEMPLATE_DIR that describes the content of the
        page.

        This content will be positioned based on the position of the
        render='content' directive of this page base template."""
        tag = context.tag.clear()
        tag[loaders.xmlfile(templateDir=config.TEMPLATE_DIR, template=self.template)]
        return tag

    def renderHTTP(self, ctx):
        """Renders the page.

        It calls the auth_http method to see if the current user has
        access to this page, if not, it sends a HTTP unauthorized response.
        """
        if self.auth_http(ctx):
            return super(BasePage, self).renderHTTP(ctx)
        else:
            request = inevow.IRequest(ctx)
            request.setHeader('WWW-Authenticate', 'Basic Realm="acpyd"')
            request.setResponseCode(http.UNAUTHORIZED)
            return "Authentication required."

    def auth_http(self, ctx):
        """Checks if the current HTTP user credentials are allowed to access
        this page."""
        request = inevow.IRequest(ctx)
        username, password = request.getUser(), request.getPassword()
        return self.allowed(username, password)

    def allowed(self, username, password):
        """Returns True if the authorizator provides access to this page. If no
        authorizator is provided, it returns True."""
        if self.authorizator is not None:
            return self.authorizator.authorize('http:/', username, password)
        else:
            return True

class ObserverMixin:
    """A mixin that provides methods to register and unregister the current
    object with the object scripted service.

    This mixin requires the object to provide a 'bench' property that provides:
        * add_observer, remove_observer
        * add_logger, remove_logger
        * add_result_handler, remove_result_handler
    If the mixin object provides (or is adapted to) ITestObserver, ITestLogger
    or ITestResultHandler, it will be registered by calling the appropiate
    method.

    It also provides a disconnect method, that is called automatically by the
    _render_* methods of the base page and base element classes, that
    unregisters the object from the scripted service.
    """
    def __init__(self):
        self.result_handler = None
        self.logger = None
        self.observer = None

    def register(self):
        """Registers the current object with it's scripted service."""
        try:
            self.observer = ITestObserver(self)
            self.bench.add_observer(self.observer)
        except exceptions.TypeError:
            self.observer = None
        try:
            self.logger = ITestLogger(self)
            self.bench.add_logger(self.logger)
        except exceptions.TypeError:
            self.logger = None
        try:
            self.result_handler = ITestResultHandler(self)
            self.bench.add_result_handler(self.result_handler)
        except exceptions.TypeError:
            self.result_handler = None

    def unregister(self):
        """Unregisters the current object from the scripted service."""
        if self.observer:
            self.bench.remove_observer(self.observer)
            self.observer = None
        if self.logger:
            self.bench.remove_logger(self.logger)
            self.logger = None
        if self.result_handler:
            self.bench.remove_result_handler(self.result_handler)
            self.result_handler = None

    def disconnected(self):
        """Handler for page disconnects that unregisters the object from the
        scripted service."""
        self.unregister()




class ReportConfig(BaseElement, ObserverMixin, Observer, Observable):
    """A widget to allow users to edit information that will go in the pdf report"""
    jsClass = u"acpyd.ReportConfig"
    
    docFactory = loaders.xmlfile(templateDir=config.TEMPLATE_DIR,
                                 template='report_config.html',
                                 pattern='ReportConfigPattern')
    def __init__(self, bench):
        BaseElement.__init__(self)
        
        #The renderer is observer and observable, needs to reflect its changes to all connected peers
        observerCollection = bench.getLiveObserver(self)
        Observer.__init__(self, observerCollection)
        Observable.__init__(self, observerCollection)
        
        #link to dict in BaseScriptedService
        self.values = bench.report_config
        self.bench = bench
        #default values if not set (if page is refreshed the bench persists but not this renderer object)
        if 'customer' not in self.values: self.values['customer'] = u''
        if 'auditor' not in self.values: self.values['auditor'] = u''
        if 'tcu' not in self.values: self.values['tcu'] = u'ACPTest'
        if 'server' not in self.values: self.values['server'] = u'ACPTest'
        if 'reference' not in self.values: self.values['reference'] = u''
        self.register()
    
    def notify(self, *args, **kwargs):
        """Update the page with new values"""
        self.callRemote('setValues', self.values['customer'], self.values['auditor'], self.values['server'], self.values['tcu'], self.values['reference'])
    
    def updateValues(self):
        """All renderers must tell their pages to update with new values (call notify in each one)"""
        self.sendNotify()
    athena.expose(updateValues)
    
    def updateReadonly(self):
        """Update if fields can be edited or not (if something is being executed it can't be changed)"""
        self.callRemote('setReadonly', self.bench.is_server_started() or self.bench.is_client_started())
    athena.expose(updateReadonly)
    
    def edit(self, key, value):
        """Inform a field update and broadcast new values to all connected peers"""
        self.values[key] = value
        self.updateValues()
    athena.expose(edit)


class ReportConfigObserver(BaseTestObserver):
    """An observer that updates the labels when an event is raised."""
    def __init__(self, form):
        BaseTestObserver.__init__(self)
        self.form = form

    def started(self, role, host):
        self.form.updateReadonly()

    def stopped(self, role, host):
        self.form.updateReadonly()

components.registerAdapter(
        ReportConfigObserver,
        ReportConfig,
        ITestObserver)

class LiveSelect(athena.LiveElement):
    """A select widget that notifies the server when it's changed."""

    jsClass = u'acpyd.LiveSelect'
    docFactory = loaders.stan(
        T.div(render=T.directive('liveElement'))[
            T.span()[T.directive('select')],
        ]
    )

    def select(self, request_, tag_):
        """Renders the select."""
        items, selected = self._get_items()

        select = T.select(onchange='Nevow.Athena.Widget.get(this).selected()')
        for name, value in items:
            if  value == selected:
                select[T.option(value=value, selected='selected')[name]]
            else:
                select[T.option(value=name)[name]]
        return select
    page.renderer(select)

    def selected(self, value):
        """Called from the browser when the select is changed.
        @param value the new value of the select element."""
        self._selected(value)
    athena.expose(selected)

    def _get_items(self):
        """Returns a (name, data) tuple of items to display on the select
        widget."""
        raise NotImplementedError()

    def _selected(self, value):
        """Called when the browser select is changed."""
        raise NotImplementedError()

class LiveLabel(athena.LiveElement):
    """A label that can be updated on the server so that is updated on
    client."""
    jsClass = u'acpyd.LiveLabel'
    docFactory = loaders.stan(
        T.div(render=T.directive('liveElement'))[
            T.span(render=T.directive('label'))
        ]
    )
    def __init__(self, text='', _class='label'):
        self.text = text
        self.cls = _class

    def getInitialArguments(self):
        """Returns the label widget initial arguments."""
        return (unicode(self.text), unicode(self.cls))

    def label(self, request_, tag_):
        """Renders the label."""
        return T.span(_class=self.cls)[self.text]
    page.renderer(label)

    def update_label(self, text, _class=None):
        """Called by the server to update the label value on the client."""
        self.text = text
        if _class is not None:
            self.cls = _class
        self.callRemote('update_label',
                        unicode(self.text),
                        unicode(self.cls))

# TODO: a etask-like live model widgets are much better than the current
# implementation, which requires two classes for each widget...
class ServiceStatusLabel(LiveLabel, ObserverMixin):
    """A label that displays the status of the service."""
    def __init__(self, bench):
        self.bench = bench
        LiveLabel.__init__(self, self.get_label())
        self.role = self.bench.role
        self.register()
    def get_label(self):
        if self.bench.is_started():
            return  u'started (%s:%s)' % (
                self.bench.ip,
                self.bench.port
            )
        else:
            return  u'stopped'

class StatusLabelObserver(BaseTestObserver):
    """An observer that updates a label when an event is raised."""
    def __init__(self, label):
        BaseTestObserver.__init__(self)
        self.label = label

    def started(self, role, host):
        if self.label.role == role:
            self.label.update_label(self.label.get_label())

    def stopped(self, role, host):
        if self.label.role == role:
            self.label.update_label(self.label.get_label())

    def connected(self, role, host, peer):
        if self.label.role == role:
            self.label.update_label(self.label.get_label())

    def disconnected(self, role, host, peer):
        if self.label.role == role:
            self.label.update_label(self.label.get_label())

    def connection_failed(self, role, host, peer):
        if self.label.role == role:
            self.label.update_label(self.label.get_label())

components.registerAdapter(
        StatusLabelObserver,
        ServiceStatusLabel,
        ITestObserver)

class ToggleButton(athena.LiveElement):
    """A toggleable button."""
    jsClass = u'acpyd.ToggleButton'
    docFactory = loaders.stan(
        T.div(render=T.directive('liveElement'))[
            T.span(render=T.directive('input'))
        ]
    )

    def __init__(self, state=True, text_up='on', text_down='off'):
        self.state = state
        self.text_up = text_up
        self.text_down = text_down

    def getInitialArguments(self):
        return self.state, self._current_value()

    def input(self, request_, tag_):
        """Renders the button."""
        return T.input(type='button',
                       _class="toggle ui-state-default fg-button-toggleable ui-corner-all",
                       value=self._current_value(),
                       onclick='Nevow.Athena.Widget.get(this).toggle()')
    page.renderer(input)

    def toggle(self):
        """Called from the client when the button is toggled."""
        self.state = not self.state
        if self.state:
            self._toggle_on()
        else:
            self._toggle_off()
        return self.state, self._current_value()
    athena.expose(toggle)

    def update_state(self, state):
        """Called from the server when the button value has changed."""
        self.state = state
        if self.fragmentParent:
            self.callRemote('update_state', self.state,
                            unicode(self._current_value()))

    def _current_value(self):
        """Returns the current value of the button."""
        if self.state:
            text = self.text_down
        else:
            text = self.text_up
        return unicode(text)

    def _toggle_on(self):
        """Override on subclasses to handle a switch from off to on."""
        raise NotImplementedError()

    def _toggle_off(self):
        """Override on subclasses to handle a switch from on to off."""
        raise NotImplementedError()

class ControlButtonObserver(BaseTestObserver):
    """An observer that updates a toggle button based on the state of a
    service."""
    def __init__(self, control):
        BaseTestObserver.__init__(self)
        self.control = control
    def started(self, role, host):
        self.control.update_state(True)
    def stopped(self, role, host):
        self.control.update_state(False)

class ServiceControlButton(ToggleButton, ObserverMixin):
    """A button that toggles the state of a service when clicked.
    The button is intialized with the role name of the service, so it can be
    later checked on ServiceControlButtonObserver""" # UGLY: too coupled
    def __init__(self, bench, text_up='start', text_down='stop'):
        ToggleButton.__init__(self,
                              state=bench.is_started(),
                              text_up=text_up,
                              text_down=text_down)
        self.bench = bench
        self.role = bench.role
        self.register()

    def _toggle_on(self):
        self.bench.add_service(
            ip=self.bench.ip,
            port=self.bench.port
        )
        self.bench.start_peer()

    def _toggle_off(self):
        self.bench.stop_peer()

    def update(self):
        self.update_state(self.bench.is_started())

    def disconnected(self):
        self.unregister()

class ServiceControlButtonObserver(ControlButtonObserver):
    """An observer that updates a toggle button based on the state of a
    service, but only if the service role matches the role of the control
    button."""
    def started(self, role, host):
        if role == self.control.role:
            self.control.update()
    def stopped(self, role, host):
        if role == self.control.role:
            self.control.update()
    def disconnected(self, role, host, peer):
        if role == self.control.role:
            self.control.update()
    def connection_failed(self, role, host, peer):
        if role == self.control.role:
            self.control.update()
components.registerAdapter(
        ServiceControlButtonObserver,
        ServiceControlButton,
        ITestObserver)

class TestbenchLog(athena.LiveElement, ObserverMixin):
    """A widget that logs test events. This widget implements ITestLogStore, so
    it can be used with observers, handlers and loggers that store events on a
    log store."""
    implements(ITestLogStore)

    jsClass = u'acpyd.TestbenchLog'
    docFactory = loaders.xmlfile(templateDir=config.TEMPLATE_DIR,
                                 template='log.html',
                                 pattern='TestbenchLogPattern')
    def __init__(self, bench, buffer_interval=1):
        self.bench = bench
        self.buffer = []
        self.last = 0
        self.timer = None
        self.buffer_interval = buffer_interval
        self.register()

    def disconnected(self):
        """Called when the page that hosts this widget is disconnected."""
        self.unregister()
        # release every reference to the service to avoid leaks
        self.bench = None

    def log(self, role, event, text, host=None, peer=None, msg=None, error=None,
            level=None, data=None):
        """
        Sends a log line to the browser.

        The widgets buffers up to  30 lines or until buffer_interval elapses
        before sending the log lines to the client.
        """
        now = datetime.datetime.utcnow()
        ts = now.utctimetuple()[:6]
        ts = ts + (now.microsecond,)

        self._buff_log(ts, role, event, text,
                       host=host, peer=host, msg=msg,
                       error=error, level=level)

        if self.buffer_interval:
            now = time.time()
            if (now - self.last) < self.buffer_interval:
                if len(self.buffer) > 30 or level == 'error':
                    self._log_flush()
                elif self.timer is None:
                    self.timer = reactor.callLater(self.buffer_interval,
                                                   self._timer_flush)
            else:
                self._log_flush()
            self.last = now
        else:
            self._log_flush()

    def _buff_log(self, ts, role, event, text, host=None, peer=None, msg=None, error=None, level=None):
        """Stores the line on the buffer, adapting it previously so it's ready
        to be sent to the browser."""
        role = unicode(role)
        event = unicode(event)
        text = unicode(text)
        if error is not None:
            error = unicode(error)
        if level is not None:
            level = unicode(level)
        if host is not None:
            host_name = unicode(host[0])
            host_port = host[1]
        else:
            host_name = u''
            host_port = 0
        if peer is not None:
            peer_name = unicode(peer[0])
            peer_port = peer[1]
        else:
            peer_name = u''
            peer_port = 0

        self.buffer.append((ts, role, event, text,
                            host_name, host_port, peer_name, peer_port,
                            msg.as_tree() if msg is not None else None,
                            error, level))

    def _timer_flush(self):
        """Called when the buffer_interval timer elapses and the lines must be
        flushed to the browser."""
        self.timer = None
        self._log_flush()

    def _log_flush(self):
        """Flushes the log lines to the browser and clears the buffer."""
        if self.fragmentParent:
            self.callRemote('log_many', self.buffer)
        self.buffer = []

# Register the TestbenchLog so it can be adapted to a log observer, logger, or
# result handler using the TestbenchLogObserver, TestbenchLogLogger and
# TestbenchLogResultHandler respectively.
components.registerAdapter(
        TestbenchLogObserver,
        TestbenchLog,
        ITestObserver)
components.registerAdapter(
        TestbenchLogLogger,
        TestbenchLog,
        ITestLogger)
components.registerAdapter(
        TestbenchLogResultHandler,
        TestbenchLog,
        ITestResultHandler)
