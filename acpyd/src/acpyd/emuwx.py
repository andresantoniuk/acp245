"""
Wx widgets based interface for testing and decoding ACP245 PDUs.
* EXPERIMENTAL *
"""
import wx
import cgi
import urllib

from twisted.web import client

import acpyd.emuwxui
import acpyd.args
from acpyd.flatten import FLATTENERS
from acpyd.args import insert_args_in_msg, insert_msg_in_args
from acpyd.script import ScriptProvider
from acp245 import pdu

class MainWindow(acpyd.emuwxui.MainFrame):
    """Main UI window."""
    def __init__(self, parent, id, title):
        """Create a new app."""
        acpyd.emuwxui.MainFrame.__init__(self, parent, id, title, size=(800,600))
        self.args = {}

        self.cb_command_arg_panel = {
            self.cb_server_commands: self.panel_server_args,
            self.cb_client_commands: self.panel_client_args,
        }
        roles_commands_cb = {
            'server': self.cb_server_commands,
            'client': self.cb_client_commands
        }
        for role, cb in roles_commands_cb.items():
            provider = ScriptProvider('%s/%s' % ('gateway_scripts', role))
            scripts = provider.get_scripts()
            for script in scripts:
                cb.Append(script.short_name, script)
                cb.Bind(wx.EVT_COMBOBOX, self.OnCommandSelect)
            if scripts:
                cb.Select(0)
                self._ShowParameters(cb)
        self.bt_server_exec.Bind(wx.EVT_BUTTON,
                                 self.CreateExecHandler(
                                     'server',self.cb_server_commands))
        self.bt_client_exec.Bind(wx.EVT_BUTTON,
                                 self.CreateExecHandler(
                                     'client', self.cb_client_commands))
        self.bt_server_start.Bind(wx.EVT_TOGGLEBUTTON, self.OnServerStartToggle)
        self.bt_client_start.Bind(wx.EVT_TOGGLEBUTTON, self.OnClientStartToggle)
        self.bt_decode.Bind(wx.EVT_BUTTON, self.OnDecodeClick)

    def OnPduPredefSelect(self, e):
        """Called when a predefined PDU is selected."""
        index = self.cb_pdu_predef.GetSelection()
        if index != wx.NOT_FOUND:
            pdu = self.cb_pdu_predef.GetClientData(index)
            self.tx_pdu_hex.SetValue(pdu)
            if pdu:
                self.OnDecodeClick(None)

    def _display_tree(self, tree, d, name='+'):
        """Displays a decoded PDU tree."""
        tree.DeleteAllItems()
        root = tree.AddRoot(name)
        self._display_tree_at(tree, root, d)
        tree.ExpandAll()

    def _display_tree_at(self, tree, root, d):
        """Displays a decoded PDU tree on the given root element."""
        for k, v in d.items():
            if isinstance(v, dict):
                newItem = tree.AppendItem(root, k)
                self._display_tree_at(tree, newItem, v)
            else:
                newItem = tree.AppendItem(root, '%s: %s' % (k, v))

    def OnDecodeClick(self, e):
        """Called when the decoded action is selected."""
        pdu_str = self.tx_pdu_hex.GetValue()
        try:
            m, l = pdu.msg_read(hex=pdu_str)
            d = m.as_dict()
            self._display_tree(self.tree_pdu, d,name=m.__class__.__name__)
        except Exception, e:
            d = wx.MessageDialog(
                self,
                "Decoding error: %s" % str(e),
                "Decode Error",
                wx.OK)
            d.ShowModal()
            d.Destroy()

    def OnServerStartToggle(self, e):
        """Called when the server toggle action is selected."""
        pressed = e.GetEventObject().Value
        if pressed:
            url = self._get_url(
                'server/start',
                {
                    'port': str(self.tx_server_port.GetValue())
                }
            )
        else:
            url = self._get_url('server/stop')
        self._request(url)

    def OnClientStartToggle(self, e):
        """Called when the client toggle action is selected."""
        pressed = e.GetEventObject().Value
        if pressed:
            url = self._get_url(
                'client/start',
                {
                    'port': str(self.tx_client_port.GetValue()),
                    'ip': str(self.tx_client_ip.GetValue())
                }
            )
        else:
            url = self._get_url('client/stop')
        self._request(url)

    def CreateExecHandler(self, role, cb):
        """Returns an event handler that performs a request against the ACP245
        gateway when the event is raised."""
        def OnExec(e):
            """The event handler."""
            index = cb.GetSelection()
            if index != wx.NOT_FOUND:
                script = cb.GetClientData(index)
                msg_args = self._get_args()
                url = self._get_url('%s/%s' %
                                    (role, script.name),
                                    msg_args)
                self._request(url,
                             msg_args=msg_args,
                             send_msg=script.send_msg,
                             recv_msg=script.recv_msg)
        return OnExec

    def _get_url(self, url_file, args={}):
        """Gets the URL to call the gateway with."""
        qs = urllib.urlencode(args)
        if '?' in url_file:
            url = '%s&%s' % (url_file, qs)
        else:
            url = '%s?%s' % (url_file, qs)
        gw_url = str(self.tx_gateway_url.GetValue())
        if gw_url.endswith('/'):
            return gw_url + 'gateway/' + url
        else:
            return gw_url + '/gateway/' + url

    def _get_args(self):
        """Get an argument value dictionary to use with the args.py module
        functions, whose values are taken from the generated window widgets."""
        args = {}
        for name, ctrl in self.args.items():
            if ctrl.GetValue():
                args[name] = str(ctrl.GetValue())
        return args

    def _request(self, url, msg_args={}, send_msg=None, recv_msg=None):
        """Performs a request to the ACP245 gateway."""
        if send_msg is not None:
            msg = send_msg()
            insert_args_in_msg(FLATTENERS[send_msg], msg_args, msg)
            self._append_log('Sending: ' + msg.as_bytes_hex())
        d = client.getPage(url)
        def print_result(result):
            """Prints the result on the log panel."""
            self._append_log(result)
            if recv_msg is not None:
                msg = recv_msg()
                msg_args = cgi.parseqs(result)
                insert_args_in_msg(FLATTENERS[recv_msg], msg_args, msg)
                self._append_log('Received: ' + msg.as_bytes_hex())
        def print_error(result):
            """Prints an error to the log panel."""
            self._append_log('Error: ' + result.getErrorMessage())
        d.addCallbacks(print_result, print_error)

    def _append_log(self, txt):
        """Logs to the log panel."""
        self.tx_result.SetValue('%s\n%s' % (self.tx_result.GetValue(), txt))

    def OnCommandSelect(self, e):
        """Called when a command is selected."""
        self._ShowParameters(e.GetEventObject())

    def _ShowParameters(self, cb):
        """Shows the message parameters, creating widgets to fill in the message
        arguments to send to the gateway based on the selected message."""
        panel = self.cb_command_arg_panel[cb]
        sizer = panel.GetSizer()
        index = cb.GetSelection()
        if index != wx.NOT_FOUND:
            script = cb.GetClientData(index)

            sizer.Clear(True)
            args = script.arguments or []
            sizer.SetRows(len(args))
            for name, fld in args:
                sizer.Add(wx.StaticText(panel, -1, name), 0,
                          wx.ADJUST_MINSIZE | wx.ALIGN_CENTER_VERTICAL,
                          0)
                param_ctrl = wx.TextCtrl(panel, -1, '')
                self.args[name] = param_ctrl
                sizer.Add(param_ctrl, 0,
                          wx.ADJUST_MINSIZE | wx.ALIGN_CENTER_VERTICAL,
                          0)
                sizer.Add(wx.StaticText(panel, -1, self._map_type(fld)), 0,
                          wx.ADJUST_MINSIZE | wx.ALIGN_CENTER_VERTICAL,
                          0)
            panel.FitInside()
            panel.Layout()

    def _map_type(self, fld):
        """Maps a type descriptor to more verbose name."""
        # This could be provided by the descriptor classes directly, avoiding
        # this map and providing the verbose name to other modules.
        type = {
            acpyd.args.Int8:      'Integer 0-255',
            acpyd.args.Int16:     'Integer 0-65535',
            acpyd.args.Int32:     'Integer 0-4294967296',
            acpyd.args.String:    'String',
            acpyd.args.Time:      'Number of seconds since epoch (01/01/1970 00:00:00).',
            acpyd.args.BinHex:    'Hexadecimal String',
        }.get(fld.__class__,'')
        if isinstance(fld, acpyd.args.List):
            type = 'comma separated list of %s' % fld.item_type.__name__
        if isinstance(fld, acpyd.args.Int):
            if fld.values:
                keys = fld.values.keys()
                keys.sort()
                type += ' or one of the following string constants: ' +\
                        ', '.join(keys)
        return type

    def OnAbout(self, e):
        """Called when the about action is selected."""
        d = wx.MessageDialog(
            self,
            "An ACP 245 Emulator - DEMO.\n(c) Edantech, 2009",
            "About ACP 245 Emulator",
            wx.OK)
        d.ShowModal()
        d.Destroy()

    def OnExit(self, e):
        """Called when exit action is selected."""
        self.Close(True)

def run_app():
    """Run the application."""
    from twisted.internet import wxreactor
    wxreactor.install()

    app = wx.PySimpleApp()
    frame = MainWindow(None, wx.ID_ANY, "ACP Emulator - DEMO")
    frame.Show(True)

    from twisted.internet import reactor
    reactor.registerWxApp(app)
    reactor.run()

if __name__ == '__main__':
    run_app()
