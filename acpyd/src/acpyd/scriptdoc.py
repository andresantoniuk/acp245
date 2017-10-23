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
"""Module to generate script documentation automatically.

To work the script should be documented accordingly.
"""
from acpyd import args
from acpyd import script
from acpyd.flatten import FLATTENERS

class FieldDoc(object):
    """Documentation of a script field."""
    def __init__(self, name, type, desc=None):
        self.name = name
        self.type = type
        self.desc = desc or ''

class CommandDoc(object):
    """Documentation of a gateway command (aka gateway script)."""
    def __init__(self, url, role, short_name, desc, args=None, resp=None):
        self.url = url
        self.role = role
        self.short_name = short_name
        self.desc = desc or ''
        self.args = args or []
        self.resp = resp or []

class DocParser:
    """Parses available scripts and builds a documentation model."""
    def parse(self, scripts_dir):
        """Parses scripts from scripts_dir and returns a documentation model."""
        cmds = [
            CommandDoc('start_test', 'bench',
                       'Start New Test',
                       "This operation signals the ACP Test Server that a new"
                       " test is starting. It should be used so the ACP Server"
                       " can later show the messages exchanged during a certain"
                       " test.",
                       [FieldDoc('name', 'String',
                                 'A descriptive name for the test')]
            ),
            CommandDoc('stop_test', 'bench',
                       'Stop current test',
                       "This operation signals the ACP Test Server that a test"
                       " has finished it's execution. It should be used so the"
                       " ACP Server knows when a given test has ended. If it's"
                       " not called, the ACP Test Server will assume that a"
                       " test ends when the next one starts."
            )
        ]
        for role in 'server', 'client':
            provider = script.ScriptProvider('%s/%s' % (scripts_dir, role))
            scripts  = provider.get_scripts()
            l = []
            for scr in scripts:
                arg_fields = []
                res_fields = []
                if scr.arguments:
                    arg_fields.extend(self._get_fields(scr.arguments))
                if scr.send_msg:
                    arg_fields.extend(self._get_fields(FLATTENERS[scr.send_msg]))
                if scr.response:
                    res_fields.extend(self._get_fields(scr.response))
                if scr.recv_msg:
                    res_fields.extend(self._get_fields(FLATTENERS[scr.recv_msg]))

                l.append(
                    CommandDoc('%s/%s' % (role, scr.name),
                               role,
                               scr.short_name,
                               scr.description,
                               arg_fields,
                               res_fields
                    )
                )
            l.sort(lambda x,y: cmp(x.short_name, y.short_name))
            if role == 'server':
                cmds.extend([
                    CommandDoc('server/start', 'server',
                             'Start Server',
                             "Starts a new server that listens to the"
                             " specified port",
                             [FieldDoc('port', 'Integer 1025-65535',
                                       'Port number')]
                    ),
                    CommandDoc('server/stop', 'server',
                             'Stop Server',
                             "Stops the running server. Clients that are"
                             " currently connected will not be disconnected"
                    )
                ])
            else:
                cmds.extend([
                    CommandDoc('client/start', 'client',
                             'Start Client',
                             "Starts a new client that connects to the"
                             " specified port",
                             [
                                FieldDoc('port', 'Integer 1025-65535',
                                         'Port number'),
                                FieldDoc('ip', 'Host name  or IP address',
                                         'Host name or IP address'),
                            ]
                    ),
                    CommandDoc('client/stop', 'client',
                             'Stop Client',
                             "Disconnects the client"
                    )
                ])
            cmds.extend(l)

        return cmds

    def _get_fields(self, flds, prefix=''):
        if flds is None:
            return []
        l = []
        for fld_name, fld in flds:
            if isinstance(fld, args.List) and fld.descriptors:
                l.append(FieldDoc('%s_cnt' % fld_name,
                                  'Integer(32)',
                                  'Number of "%s" elements' % fld_name))
                l.extend(self._get_fields(fld.descriptors, prefix='%s_<n>_' % fld_name))
            else:
                l.append(FieldDoc(fld_name,
                                  self._map_type(fld),
                                  getattr(fld, 'note', '')))
        return l

    def _map_type(self, fld):
        type = {
            args.Int8:      'Integer 0-255',
            args.Int16:     'Integer 0-65535',
            args.Int32:     'Integer 0-4294967296',
            args.String:    'String',
            args.Time:      'Number of seconds since epoch (01/01/1970 00:00:00).',
            args.BinHex:    'Hexadecimal String',
        }.get(fld.__class__,'')
        if isinstance(fld, args.List):
            type = 'comma separated list of %s' % fld.item_type.__name__
        if isinstance(fld, args.Int):
            if fld.values:
                keys = fld.values.keys()
                keys.sort()
                type += ' or one of the following string constants: ' +\
                        ', '.join(keys)
        return type
class HtmlGenDoc:
    """A generator from the documentation model to HTML."""
    def gen(self, cmds):
        """Generates and returns the HTML.
        @param cmds the documentation model."""
        html = """
<html>
<head>
    <title>ACP245 Gateway Commands Reference</title>
<style type="text/css">
table {
    width: 35em;
    padding: 0;
    border-collapse: collapse;
    margin: 0;
    margin-top: 3em;
    border: 1px solid black;
}
th {
    border: 1px solid black;
    font-weight: bold;
    text-align: left;
}
th.hdr-short-name {
    width: 15em;
}
th.hdr-desc {
    width: 15em;
}
tr th.fld-hdr {
    text-align: center;
}
th.name {
    width: 15em;
}
th.type {
    width: 10em;
}
th.notes {
    width: 10em;
}
tr {
    border: 1px solid black;
    margin: 0;
}
td {
    border: 1px solid black;
    margin: 0;
}
td.url {
    font-family: monospace;
}
</style>
</head>
<body>
"""
        html += self._gen_cmd(cmds)
        html += '</body></html>'
        return html

    def _gen_cmd(self, cmds):
        html = ''
        for cmd in cmds:
            html += """
<table width="500px" style="border: 1px solid black;">
<tr><th class='hdr-shortname' colspan='3'>%(short_name)s</th></tr>
<tr><th class='hdr-url' width="150px">URL</th><td colspan='2' class='url'style='font-family: monospace;'>gateway/%(url)s</td></tr>
<tr><th class='hdr-desc' width="150px">Description</th><td colspan='2'>%(desc)s</td></tr>
""" % {
    'short_name': cmd.short_name,
    'url': cmd.url,
    'desc': cmd.desc
}
            if cmd.args:
                html += """
<tr><th colspan='3'>Parameters</th></tr>
"""
                html += self._gen_fields(cmd.args)
            if cmd.resp:
                html += """
<tr><th colspan='3'>Response</th></tr>
"""
                html += self._gen_fields(cmd.resp)
            html += """
</table>
<br/>
<br/>
"""
        return html
    def _gen_fields(self, fields):
        html = ''
        html += """
<tr class='fld-hdr'>
    <th class='name' width="200px">Name</th>
    <th class='type' width="150px">Type</th>
    <th class='notes' width="100px">Notes</th>
</tr>
"""
        for arg in fields:
            html += """
<tr>
    <td style='font-family: monospace;'>%(name)s</td>
    <td>%(type)s</td>
    <td>%(notes)s</td>
</tr>
""" % {
    'name': arg.name,
    'type': arg.type,
    'notes': arg.desc,
}
        html += """
"""
        return html
