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
"""Exports TestDB log to PDF report."""

import os
import time
import StringIO
import logging
from xml.sax import saxutils

from zope.interface import implements

from axiom.attributes import AND, OR
from twisted.python import components
from twisted.internet import threads

from reportlab.platypus.doctemplate import (
    BaseDocTemplate, PageTemplate, SimpleDocTemplate, Indenter
)
from reportlab.platypus.paragraph import Paragraph
from reportlab.platypus.frames import Frame
from reportlab.platypus.tables import Table
from reportlab.platypus.flowables import (
    Flowable, Spacer, PTOContainer,
    HRFlowable, KeepInFrame, PageBreak
)
from reportlab.lib import units, colors, enums, styles

from acp245 import pdu

from acpyd import config
from acpyd.testobservers import (
    SENT_MSG,
    RECV_MSG,
    TEST_STARTED,
    TEST_PASSED,
    TEST_FAILED
)
from acpyd.protocol import CLIENT_ROLE, SERVER_ROLE

from acpyd.testdb import TestDb, TestLogLine
from acpyd.testdb.interfaces import IPDFTestReport

log = logging.getLogger()

__all__ = ['PDFTestReport']

#
# Define global PDF Styles
#

e = saxutils.escape

stylesheet = styles.getSampleStyleSheet ()
h1 = stylesheet['h1']
h2 = stylesheet['h2']
h3 = stylesheet['h3']
normal = stylesheet['Normal']
title = stylesheet['Title']
script_title = styles.ParagraphStyle(
    name='script_title',
    parent=h1,
    alignment=enums.TA_CENTER,
)
pto_title_header = styles.ParagraphStyle(
    name='pto_title_header',
    parent=normal,
    fontSize=12,
    spaceAfter=6,
    alignment=enums.TA_CENTER,
)
pto_title_trailer = styles.ParagraphStyle(
    name='pto_title_trailer',
    parent=normal,
    fontSize=12,
    spaceBefore=6,
    alignment=enums.TA_CENTER,
)
centered = styles.ParagraphStyle(
    name='centered',
    parent=normal,
    alignment=enums.TA_CENTER,
)
color_edantech = colors.HexColor(0x3B70A0)

TYPE_NAMES={
    'AlarmKA': 'Keep Alive',
    'AlarmKAReply': 'Keep Alive Reply',
    'AlarmNotif': 'Theft Alarm Notification',
    'AlarmPos': 'Theft Alarm Position',
    'AlarmReply': 'Theft Alarm Reply',
    'CfgActivation': 'TCU Service Activation/Deactivation', 
    'CfgReply': 'Configuration Reply',
    'CfgReply245': ' Configuration Reply #2 ACP 245',
    'CfgUpd245': 'Configuration Update #2 ACP 245',
    'FuncCmd': 'Vehicle Function Command',
    'FuncStatus': 'Vehicle Function Status',
    'IEAny': 'Information Element',
    'IEApnCfg': 'APN Configuration',
    'IEBreakdownStatus': 'Breakdown Status',
    'IECtrlFunc': 'Control Function',
    'IEDeadReck': 'Dead Reckoning',
    'IEError': 'Error',
    'IEFuncCmd': 'Function Command',
    'IEGPSRawData': 'GPSRawData',
    'IEInfoType': 'Information Type',
    'IELocDelta': 'Array of Location Delta Coding',
    'IELocDeltaLatlon': 'Location Delta Lat/Lon',
    'IELocation': 'Location',
    'IERawData': 'Raw Data',
    'IEServerCfg': 'Server Configuration',
    'IETCUData': 'TCU Data Array',
    'IETCUDataError': 'TCU Data Error Array',
    'IETCUDataErrorItem': 'TCU Data Error',
    'IETCUDataItem': 'TCU Data',
    'IETCUDesc': 'TCU Descriptor',
    'IETimestamp': 'Timestamp',
    'IEVehicleDesc': 'Vehicle Descriptor',
    'IEVersion': 'Version',
    'ProvReply': 'Provision Reply',
    'ProvUpd': 'Provision Update',
    'TrackCmd': 'Vehicle Tracking Command',
    'TrackPos': 'Vehicle Position',
    'TrackReply': 'Vehicle Position Reply',
}

def get_type_name(el):
    return TYPE_NAMES.get(
        el.__class__.__name__,
        el.__class__.__name__,
    )

def __generic_element_formatter(name, el):
    story = []
    story.append(Paragraph(e('%s' % name), normal))
    format_fields_decoded(story, el, 'present')
    return story

def __timestamp_formatter(name, el):
    return [Paragraph('%s = %s' % (
        name,
        time.strftime('%Y/%m/%d %H:%M:%S', time.gmtime(el.get_time()))
    ), normal)]

def __vehicle_desc_formatter(name, el):
    story = []
    story.append(Paragraph(
        e(
            name + '=' +
            (' ICCID: %s' % el.iccid if el.iccid is not None else '') +
            (' flag1: 0x%.2X' % el.flg1) +
            (' flag2: 0x%.2X' % el.flg2 if el.flg2 is not None else '')
        ),
        normal
    ))
    format_fields_decoded(story, el, 'present', 'iccid', 'flg1', 'flg2')
    return story

def __version_formatter(name, el):
    return [Paragraph(
        '%s = car manuf.: %d TCU manuf.: %d hardware release: %d software release: %d' % (
        name,
        el.car_manufacturer,
        el.tcu_manufacturer,
        el.major_hard_rel,
        el.major_soft_rel
    ), normal)]

def __location_formatter(name, el):
    if (el.curr_gps is None and
        el.prev_gps is None and
        el.dead_reck is None and
        el.loc_delta is None):
        return [Paragraph(e('%s (empty)' % name), normal)]
    else:
        return __generic_element_formatter(name, el)

def __tcu_data_error_item_formatter(name, el):
    return [Paragraph(e('%s = type: 0x%.4X error: %d data: %s' % (
        name,
        el.type,
        el.error.code,
        '0x%s' % ''.join(['%.2X' % x for x in el.data]) if el.data else '(empty)'
    )), normal)]

def __tcu_data_item_formatter(name, el):
    return [Paragraph(e('%s = type: 0x%.4X data: %s' % (
        name,
        el.type,
        '0x%s' % ''.join(['%.2X' % x for x in el.data]) if el.data else '(empty)'
    )), normal)]

def __info_type_formatter(name, el):
    return [Paragraph(e('%s = type: 0x%.4X data: %s' % (
        name,
        el.type,
        '0x%s' % ''.join(['%.2X' % x for x in el.data]) if el.data else '(empty)'
    )), normal)]

def __error_formatter(name, el):
    return [Paragraph(e('%s = code: %d' % (
        name,
        el.code,
    )), normal)]

def __gps_raw_data_formatter(name, el):
    story = []
    story.append(Paragraph(
        e('%s = Lat./Lon.: %s (degrees)' % (name, '%.6f, %.6f' % el.coords)),
        normal
    ))
    format_fields_decoded(story, el, 'present')
    return story

def __func_cmd_formatter(name, el):
    story = []
    story.append(Paragraph(
        e('%s = Command: %d (%s)' % (
            name, el.cmd,
            {
                pdu.ACP_FUNC_CMD_PERMIT: 'PERMIT',
                pdu.ACP_FUNC_CMD_REJECT: 'REJECT',
                pdu.ACP_FUNC_CMD_ENABLE: 'ENABLE',
                pdu.ACP_FUNC_CMD_DISABLE: 'DISABLE',
                pdu.ACP_FUNC_CMD_REQUEST: 'REQUEST',
            }.get(el.cmd, 'UNKNOWN')
        )),
        normal
    ))
    format_fields_decoded(story, el, 'present', 'cmd')
    return story

def __ctrl_func_formatter(name, el):
    story = []
    story.append(Paragraph(
        e('%s = Entity ID: %d' % (name, el.entity_id)),
        normal
    ))
    format_fields_decoded(story, el, 'present', 'entity_id')
    return story

FORMATTERS={
    pdu.IETimestamp: __timestamp_formatter,
    pdu.IEVersion: __version_formatter,
    pdu.IELocation: __location_formatter,
    pdu.IETCUDataErrorItem: __tcu_data_error_item_formatter,
    pdu.IETCUDataItem: __tcu_data_item_formatter,
    pdu.IEInfoType: __info_type_formatter,
    pdu.IEError: __error_formatter,
    pdu.IEGPSRawData: __gps_raw_data_formatter,
    pdu.IEVehicleDesc: __vehicle_desc_formatter,
    pdu.IEFuncCmd: __func_cmd_formatter,
    pdu.IECtrlFunc: __ctrl_func_formatter,
}
def get_element_formatter(el):
    return FORMATTERS.get(
        el.__class__,
        __generic_element_formatter
    )

def break_text(text, size):
    return [text[i:i+size] for i in range(0, len(text), size)]

def get_msg(line, breakat=36):
    pdu = line.pdu()
    if pdu is not None:
        return [get_type_name(pdu)] + break_text(pdu.as_bytes_hex(),
                                                 breakat)
    else:
        return line.msg,

def build_flow_table(test):
    lines = list(test.store.query(TestLogLine, TestLogLine.test == test))
    flow = [('Time', 'TCU', 'Service Operator')]
    for line in lines:
        type = line.event, line.role
        date = '%s' % time.strftime('%H:%M:%S', line.date.asStructTime())
        if type in ((SENT_MSG, SERVER_ROLE), (RECV_MSG, CLIENT_ROLE)):
            tcu, so = '', [Paragraph(x, normal) for x in get_msg(line)]
        elif type in ((RECV_MSG, SERVER_ROLE), (SENT_MSG, CLIENT_ROLE)):
            tcu, so = [Paragraph(x, normal) for x in get_msg(line)], ''
        else:
            continue
        flow.append((Paragraph(e(date), normal), tcu, so))

    if test.passed():
        flow.append(('Test Result', '', 'Passed'))
    else:
        flow.append(('Test Result', '', 'Failed'))

        # First the top row, with all the text centered and in Times-Bold,
        # and one line above, one line below.
    ts = [
        ('ALIGN', (0,1), (1,-1), 'CENTER'),
        ('VALIGN', (0,1), (1,-1), 'MIDDLE'),
        ('LINEBELOW', (0,1), (-1,-2), 0.5, colors.grey),
        ('ALIGN', (1,1), (-1,-1), 'LEFT'),
        ('LINEABOVE', (0,0), (-1,0), 1, color_edantech),
        ('LINEBELOW', (0,0), (-1,0), 1, color_edantech),
        ('FONT', (0,0), (-1,0), 'Times-Bold'),
        # The bottom row has one line above, and three lines below of
        # various colors and spacing.
        ('LINEABOVE', (0,-1), (-1,-1), 1, color_edantech),
        ('LINEBELOW', (0,-1), (-1,-1), 0.5, color_edantech,
         1, None, None, 4,1),
        ('LINEBELOW', (0,-1), (-1,-1), 1, colors.red),
        ('FONT', (0,-1), (-1,-1), 'Times-Bold')
         ]

    return [
        PTOContainer(
            Table(flow, style=ts, colWidths=(20*units.mm, '*','*')),
            trailer=Paragraph('Continues on next page', pto_title_trailer),
            header=Paragraph('Continued from previous page', pto_title_header),
        )
    ]

def build_sequence_diagram(test):
    lines = list(test.store.query(
        TestLogLine,
        AND(
            TestLogLine.test == test,
            OR(
                TestLogLine.event == SENT_MSG,
                TestLogLine.event == RECV_MSG
            )
        )
    ))
    rows = []
    tcu_addr = None
    tiv_addr = None
    for line in lines:
        type = line.event, line.role
        date = '%s' % time.strftime('%H:%M:%S', line.date.asStructTime())
        if type in ((SENT_MSG, SERVER_ROLE), (RECV_MSG, CLIENT_ROLE)):
            dir = Arrow.LEFT
        elif type in ((RECV_MSG, SERVER_ROLE), (SENT_MSG, CLIENT_ROLE)):
            dir = Arrow.RIGHT
        else:
            continue

        if line.peer_name and not tcu_addr:
            if line.role == SERVER_ROLE:
                tcu_addr = '%s:%s' % (line.peer_name, line.peer_port)
                tiv_addr = '%s:%s' % (line.host_name, line.host_port)
            else:
                tcu_addr = '%s:%s' % (line.host_name, line.host_port)
                tiv_addr = '%s:%s' % (line.peer_name, line.peer_port)

        msg = line.pdu()
        if msg is not None:
            name = Paragraph(get_type_name(line.pdu()), centered)
            binary = [Paragraph(x, centered) for x in break_text(line.pdu().as_bytes_hex(), 46)]
        else:
            name = Paragraph(line.msg, centered)
            binary = []

        rows.append((
            Paragraph(e(date), normal),
            Paragraph('TCU', centered),
            KeepInFrame('60%','80%', [
                    name,
                    Arrow(width=120*units.mm, dir=dir, spaceBefore=0),
                ] + binary,
            ),
            Paragraph('TIV', centered),
        ))

    return [
        PTOContainer(
            Table([(
                'Time',
                KeepInFrame('100%','100%', [
                    Paragraph('TCU', centered),
                ] + ([Paragraph(tcu_addr, centered)] if tcu_addr else [])
                ),
                'Message',
                KeepInFrame('100%','100%', [
                    Paragraph('TIV', centered)
                ] + ([Paragraph(tiv_addr, centered)] if tiv_addr else [])
                ),
            )] + rows,
            colWidths=(20*units.mm, '*', 100*units.mm, '*'),
            style=[
                ('LINEBELOW', (0, 0), (-1, 0), 1, color_edantech),
                ('ALIGN', (0, 0), (-1, -1), 'CENTER'),
                ('VALIGN', (0, 0), (-1, -1), 'TOP'),
                ('FONT', (0, 0), (-1, 0), 'Times-Bold')
            ]),
            trailer=Paragraph('Continues on next page', pto_title_trailer),
            header=Paragraph('Continued from previous page', pto_title_header),
        )
    ]

def format_fields_decoded(story, el, *ignore_fields):
    fields = [(x, getattr(el,x))
              for x in el.get_fields()
              if x not in ignore_fields and getattr(el, x) is not None]
    for (k, v) , i in zip(fields, range(len(fields))):
        build_decoded(story, k, v, last=(i == len(fields) - 1))
    return story

def build_decoded(story, name, v, last=False):
    if isinstance(v, pdu.Element):
        content = []
        type_name = get_type_name(v)
        if getattr(v, 'present', None) == 0:
            content.append(Paragraph(e('%s (empty)' % name), normal))
        else:
            content.extend(get_element_formatter(v)(name, v))
    elif isinstance(v, list):
        content = []
        content.append(Paragraph(e('%s' % name), normal))
        for i in range(len(v)):
            build_decoded(content, str(i), v[i], last=(i == len(v) - 1))
    else:
        content = [Paragraph(e('%s = %s' % (name, v)), normal)]

    story.append(Table(
        [(BranchBullet(12,12, last=last), KeepInFrame('80%', '80%', content))],
        style=(
            ('VALIGN', (0, 0), (-1, -1), 'TOP'),
            ('TOPPADDING', (0, 0), (-1, 0), 0),
            ('BOTTOMPADDING', (0, 0), (-1, 0), 0),
            ('LEADING', (0, 0), (-1, 0), 0),
        ),
        colWidths=(5*units.mm, '*'),
    ))
    return story


def build_test_details(test):
    story = []
    story.append(Paragraph('Test detail', title))
    story.append(Spacer(1, 0.5 * units.cm))

    if test.bench_version > 1:
        lines = list(test.store.query(
            TestLogLine,
            AND(
                TestLogLine.test == test,
                TestLogLine.level != u'debug'
            )
        ))
    else:
        lines = list(test.store.query(
            TestLogLine,
            TestLogLine.test == test,
        ))

    for line in lines:
        event = line.event
        text = e(line.msg)
        msg = line.pdu()

        if msg:
            if event == RECV_MSG:
                event = 'Received from %s:%d' % (line.peer_name, line.peer_port)
            elif event == SENT_MSG:
                event = 'Sent to %s:%d' % (line.peer_name, line.peer_port)

        if event == TEST_STARTED:
            event = 'Started test'
            text = e("'%s'" % test.name)
        elif event == TEST_PASSED:
            event = ''
            text = '<font color="green">%s</font>' % e(text)
        elif event == TEST_FAILED:
            event = ''
            text = '<font color="red">%s</font>' % e(text)

        story.append(Paragraph('<b>%s - %s <i>%s</i><b>' % (
                e(time.strftime('%H:%M:%S', line.date.asStructTime())),
                e(event),
                e(get_type_name(msg)) if msg else text,
            ), normal))

        if line.event in (RECV_MSG, SENT_MSG) and msg:
            story.extend([Paragraph(x, normal) for x in
                          break_text(msg.as_bytes_hex(), 100)])
            format_fields_decoded(story, msg)
    return story

class ReportPageTemplate(PageTemplate):
     def __init__(self, id, pageSize, title):
         self.numPages = 0
         self.pageWidth = pageSize[0]
         self.pageHeight = pageSize[1]
         self.title = title
         frame1 = Frame(20*units.mm,
                        15*units.mm,
                        self.pageWidth - 40*units.mm,
                        self.pageHeight - 35*units.mm, id='normal')
         PageTemplate.__init__(self, id, [frame1])

     def afterDrawPage(self, canvas, doc):
        y = self.pageHeight - 15*units.mm
        canvas.saveState()
        company_logo = os.path.join(
            config.STATIC_DIR, 'css', 'images', 'company.gif'
        )
        if os.path.isfile(company_logo):
            canvas.drawImage(
                os.path.join(
                config.STATIC_DIR, 'css', 'images', 'logo.gif'
            ), 20*units.mm, self.pageHeight-13*units.mm)
            title_margin = 60
        else:
            title_margin = 20
        canvas.setFont('Helvetica', 12)

        canvas.drawString(title_margin*units.mm, y+8, self.title)
        canvas.line(20*units.mm, y, self.pageWidth - 20*units.mm, y)
        canvas.setFont('Helvetica', 8)
        canvas.drawRightString(
            self.pageWidth - 20*units.mm, y+8,
            "Page %(page)d of %(numpages)d" % {
                "page": canvas.getPageNumber(),
                "numpages": doc.numPages
            }
        )

        edantech_logo = os.path.join(
            config.STATIC_DIR, 'css', 'images', 'logo.gif'
        )
        footer_baseline = 8 * units.mm
        canvas.setFont('Helvetica', 8)
        canvas.drawRightString(
            self.pageWidth - 41*units.mm,
            footer_baseline + 1.5*units.mm,
            "Generated by ACP245 Test Server, powered by "
        )
        canvas.drawImage(
            edantech_logo,
            self.pageWidth - 40*units.mm,
            footer_baseline
        )

        canvas.restoreState()


class StandardDocTemplate(BaseDocTemplate):

    def addProgressBarCallback(self, func_progressbar_callback):
        self.func_progressbar_callback = func_progressbar_callback
     
    def progresshandler(self, what, arg):
        if what == 'STARTED':
            self._lastnumPages = self.numPages
        if what == 'PASS':
            self.pass_idx = arg
        if what == 'SIZE_EST':
            self.doc_size= arg
        if what == 'PROGRESS':
            if self.pass_idx == 1:
                self.progress = (arg*100) / (self.doc_size*2)
            else:
                self.progress = ((arg*100) / (self.doc_size*2)) + 50
            self.func_progressbar_callback(self.progress)
    
    def afterInit(self):
        self.addPageTemplates(ReportPageTemplate(
             'report-page', self.pagesize,
             self.title
        ))

        #just playing
        self.numPages = 1
        self._lastnumPages = 0
        self.setProgressCallBack(self.progresshandler)

    def afterPage(self):
        """This is called after page processing, and
        immediately after the afterDrawPage method
        of the current page template."""
        self.numPages = max(self.canv.getPageNumber(), self.numPages)

    def _allSatisfied(self):
        if self._lastnumPages < self.numPages:
            return 0
        return BaseDocTemplate._allSatisfied(self)

class BranchBullet(Flowable):
    def __init__(self, w, h, last=False):
        Flowable.__init__(self)
        self.width = w
        self.height = h
        self.last = last

    def draw(self):
        canv = self.canv
        canv.saveState()
        canv.setLineWidth(1)
        canv.setLineCap(0) # but
        canv.setStrokeColor(color_edantech)
        corner_h = int(self.height*0.35)
        corner_w = self.width/2
        canv.line(corner_w, self.height, corner_w,
                  corner_h if self.last else 0)
        canv.line(corner_w, corner_h, self.width, corner_h)
        canv.restoreState()

class Arrow(HRFlowable):
    LEFT = 1
    RIGHT = 2

    def __init__(self, dir=RIGHT, **kwargs):
        HRFlowable.__init__(self, **kwargs)
        assert dir in (Arrow.LEFT, Arrow.RIGHT)
        self.dir = dir

    def draw(self):
        canv = self.canv
        canv.saveState()
        canv.setLineWidth(self.lineWidth)
        canv.setLineCap({'butt':0,'round':1, 'square': 2}[self.lineCap.lower()])
        canv.setStrokeColor(self.color)
        if self.dash: canv.setDash(self.dash)
        canv.line(6, 0, self._width - 6, self.height)
        if self.dir == Arrow.LEFT:
            canv.lines([
                (0,0, 6, 6),
                (6, 6, 6, -6),
                (6, -6, 0, 0)
            ])
        else:
            canv.lines([
                (self._width, 0, self._width - 6, 6),
                (self._width - 6, 6, self._width - 6, -6),
                (self._width - 6, -6, self._width, 0)
            ])
        canv.restoreState()

class PDFTestReport(object):
    """A PDF report of test runs."""
    implements(IPDFTestReport)
    def __init__(self, store):
        self.store = store

    def get_pdf(self, test_ids, output=None, progressbar_callback=None):
        """Creates a PDF report for the given test IDs.
        If output is provided, the report will be written to output. Otherwise,
        the function will return a binary string with the PDF output.
        """

        if output is None:
            output = StringIO.StringIO()

        # define styles

        # Document generation
        doc = StandardDocTemplate(
            output,
            title='ACP245 Test Results',
            author='Generated by Edantech ACP245 Test Server',
        )
        doc.addProgressBarCallback(progressbar_callback)
        story = []
        for test_id, i in zip(test_ids, range(len(test_ids))):
            test = self.store.get_test(test_id)
            self._get_test_pdf(test, story)
            if i != len(test_ids) - 1:
                story.append(PageBreak())

        def done(result):
            log.debug('PDF built')
            if isinstance(output, StringIO.StringIO):
                return output.getvalue()
        log.debug('building PDF')
        return threads.deferToThread(doc.multiBuild, story).addCallback(done)

    def _get_test_pdf(self, test, story):
        if test.name.startswith('%s_' % CLIENT_ROLE):
            name = test.name[len('%s_' % CLIENT_ROLE):]
            side = 'TCU'
        elif test.name.startswith('%s_' % SERVER_ROLE):
            name = test.name[len('%s_' % SERVER_ROLE):]
            side = 'TIV'
        else:
            name = test.name
            side = ''

        story.append(Paragraph('%s Script: %s' % (side, e(name)), script_title))
        if(test.customer != ''):
                story.append(Paragraph('<b>Customer: </b>%s' % e(test.customer), normal))
        if(test.auditor != ''):
                story.append(Paragraph('<b>Auditor: </b>%s' % e(test.auditor), normal))
        story.append(Spacer(1, 0.3 * units.cm))
        if(test.tcu != ''):
                story.append(Paragraph('<b>TCU: </b>%s' % e(test.tcu), normal))
        if(test.server != ''):
                story.append(Paragraph('<b>Server: </b>%s' % e(test.server), normal))
        story.append(Spacer(1, 0.3 * units.cm))
        if(test.reference != ''):
                story.append(Paragraph('<b>Reference: </b>%s' % e(test.reference), normal))
        story.append(Paragraph(
            '<b>Date: </b>%s' % e(time.strftime(
                '%Y/%m/%d %H:%M:%S',
                test.date.asStructTime()
            )
        ), normal))
        if test.passed():
            result = '<b>Result: <font color="green">PASSED</font></b>'
        else:
            result = '<b>Result: <font color="red">FAILED</font></b>'
        story.append(Paragraph(result, normal))

        story.append(Spacer(1, 0.5 * units.cm))

        story.append(Paragraph('Message flow', title))

        #story.extend(build_flow_table(test))
        #story.append(Spacer(1, 0.5 * units.cm))
        story.extend(build_sequence_diagram(test))
        story.append(Spacer(1, 0.5 * units.cm))
        story.extend(build_test_details(test))
        return story

def register():
    components.registerAdapter(
        PDFTestReport,
        TestDb,
        IPDFTestReport
    )
