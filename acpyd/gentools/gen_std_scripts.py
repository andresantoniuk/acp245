import sys
import os

from Cheetah.Template import Template

from acp245.pdu import *
from acp245.stdmsg import STANDARD_MESSAGES


outdir = sys.argv[1]
exec_dir = os.path.split(os.path.abspath(sys.argv[0]))[0]
send_tmpl = Template.compile(file=os.path.join(exec_dir, 'send_script.tmpl'))
recv_tmpl = Template.compile(file=os.path.join(exec_dir, 'recv_script.tmpl'))
seen_sections = {}
for section in STANDARD_MESSAGES:
    rq = section['messages'][0]
    if len(section['messages']) > 1:
       rp = section['messages'][1]
    else:
        rp = None

    if rq:
        request = msg_read(hex=rq['hex'])[0]
    else:
        request = None

    if rp:
        reply = msg_read(hex=rp['hex'])[0]
    else:
        reply = None

    seen_sections[section['section']] = seen_sections.get(section['section'], 0) + 1
    if seen_sections[section['section']] > 1:
        section_fname_prefix = '%s-%d' % (section['section'],
                                          seen_sections[section['section']])
    else:
        section_fname_prefix = section['section']

    for role in ('server', 'client'):
        if request is not None and request.__class__ in \
           (ProvUpd, CfgUpd245, CfgUpd245, CfgActivation,
            FuncCmd, TrackCmd, TrackReply, AlarmReply, AlarmKAReply):
            if role == 'server':
                tmpl = send_tmpl
            else:
                tmpl = recv_tmpl
        else:
            if role == 'client':
                tmpl = send_tmpl
            else:
                tmpl = recv_tmpl

        f = open(
            os.path.join(
                outdir,
                '%s_standard_%s.py' % (role, section_fname_prefix)
            ),
            'w'
        )

        def hex_f(x):
            s = hex(int(x)).upper()[2:]
            if len(s) % 2 == 0:
                return '0x%s' % s
            else:
                return '0x0%s' % s

        namespace = {
            'common_lib': os.path.join(exec_dir, 'common_script.tmpl'),
            'hex': hex_f,
            'description': '%s %s' % (
                section['section'],
                section['title']
            ),
            'shortname': '%s %s' % (
                section['section'],
                section.get('shortname',section['title'])
            ),
            'role': role,
        }
        if rq is not None:
            namespace.update({
                'request_hex': rq['hex'],
                'request_section': '%s%s' % (section['section'], rq['section']),
            })
        namespace['req'] = request
        if request is not None:
            namespace['request_type'] = request.__class__.__name__
            namespace['request_has_vehicle_desc'] = \
                    hasattr(request, 'vehicle_desc') and \
                    request.vehicle_desc is not None and \
                    request.vehicle_desc.present
            namespace['request_has_version'] = \
                    hasattr(request, 'version') and \
                    request.version is not None and \
                    request.version.present


        if rp is not None:
            namespace.update({
                'reply_hex': rp['hex'],
                'reply_section': '%s%s' % (section['section'], rp['section']),
            })
        namespace['rep'] = reply
        if reply is not None:
            namespace['reply_type'] = reply.__class__.__name__
            namespace['reply_has_vehicle_desc'] = \
                    hasattr(reply, 'vehicle_desc') and \
                    reply.vehicle_desc is not None and \
                    reply.vehicle_desc.present
            namespace['reply_has_version'] = \
                    hasattr(reply, 'version') and \
                    reply.version is not None and \
                    reply.version.present
        def has_el(el,name):
            return el is not None and hasattr(el, name) and getattr(el, name) not in (None, 0)
        namespace['has_el'] = has_el
        out = tmpl(namespaces=namespace)
        f.write(str(out))

        f.close()
