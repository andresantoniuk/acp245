from zope.interface import implements

from acpyd.testdb.interfaces import IPDFTestReport, IKMLTestReport

class PDFTestReport(object):
    implements(IPDFTestReport)
    def __init__(self, test):
        self.test = test

    def get_pdf(self, output=None):
        if output is None:
            output = StringIO.StringIO()

        styles = getSampleStyleSheet ()
        h1 = styles["h1"]
        normal = styles["Normal"]
        title = styles["Normal"]

        doc = SimpleDocTemplate (output)
        test = self.test
        story = []
        logs = list(test.store.query(TestLogLine, TestLogLine.test == test))
        e = saxutils.escape
        story.append(Paragraph(e('Script: %s' % test.name), h1))

        story.append(Paragraph(e(
            'Date: %s' % time.strftime(
                '%Y/%m/%d %H:%M:%S',
                test.date.asStructTime()
            )
        ), normal))

        story.append(Spacer(1, 0.5 * units.cm))

        story.append(Paragraph('Message flow', title))


        flow = [('TCU', 'Service Operator')]
        for log in logs:
            if log.event == 'sent_msg':
                if log.role == 'server':
                    flow.append(('',log.pdu().__class__.__name__))
                else:
                    flow.append((log.pdu().__class__.__name__, ''))
            elif log.event == 'received_msg':
                if log.role == 'client':
                    flow.append(('',log.pdu().__class__.__name__))
                else:
                    flow.append((log.pdu().__class__.__name__, ''))

        if test.passed():
            flow.append(('Test Result', 'Passed'))
        else:
            flow.append(('Test Result', 'Failed'))

            # First the top row, with all the text centered and in Times-Bold,
            # and one line above, one line below.
        ts = [('ALIGN', (1,1), (-1,-1), 'CENTER'),
             ('LINEABOVE', (0,0), (-1,0), 1, colors.purple),
             ('LINEBELOW', (0,0), (-1,0), 1, colors.purple),
             ('FONT', (0,0), (-1,0), 'Times-Bold'),
            # The bottom row has one line above, and three lines below of
            # various colors and spacing.
             ('LINEABOVE', (0,-1), (-1,-1), 1, colors.purple),
             ('LINEBELOW', (0,-1), (-1,-1), 0.5, colors.purple,
              1, None, None, 4,1),
             ('LINEBELOW', (0,-1), (-1,-1), 1, colors.red),
             ('FONT', (0,-1), (-1,-1), 'Times-Bold')]
        story.append(Table(flow, style=ts))


        story.append(Spacer(1, 0.5 * units.cm))

        for log in [x for x in logs if x.level != 'debug']:
            story.append(
                Paragraph(
                    e('%(date)s %(role)s %(event)s: %(msg)s' % {
                        'date': time.strftime('%H:%M:%S',
                                              log.date.asStructTime()),
                        'role': log.role,
                        'event': log.event,
                        'msg': log.msg
                    }),
                    normal
                )
            )
            if log.pdu() is not None:
                def print_dict(story, d, depth):
                    story.append(Paragraph(e('%s' % d[0]), normal))
                    story.append(Indenter(left = (depth+1) * units.cm))
                    for key, value in d[1:]:
                        if isinstance(value, tuple) and value and isinstance(value[0], str):
                            story.append(Paragraph(e('%s:' % key), normal))
                            print_dict(story, value, depth+1)
                        else:
                            story.append(Paragraph(e('%s = %s' % (key, value)), normal))
                    story.append(Indenter(right = (depth+1) * units.cm))
                print_dict(story, log.pdu().as_tree(), 0)
        doc.build(story)
        return output

class KMLTestReport(object):
    implements(IKMLTestReport)
    def KMLTestReport(self, test):
        self.test = test

    def get_kml(self):
        logs = list(self.test.store.query(
            TestLogLine,
            TestLogLine.test == self.test
        ))
        coords = []
        for log in logs:
            pdu_dict = log.pdu_dict()
            if pdu_dict is not None:
                location = pdu_dict.get('location')
                if location is not None:
                    gps = location.get('curr_gps')
                    if gps is not None:
                        coord = gps.get('coords')
                        if coord is not None:
                            alt = gps.get('alt', 0)
                            # invert lon,lat to lat,lon,alt
                            coords.append((float(coord[1]), float(coord[0]), int(alt)))
        return '''<?xml version="1.0" encoding="UTF-8"?>
<kml xmlns="http://www.opengis.net/kml/2.2">
  <Document>
    <name>%(testname)s</name>
    <description>
        Positions received under test %(testname)s on %(date)s
    </description>
    <Style id="yellowLineGreenPoly">
      <LineStyle>
        <color>7f00ffff</color>
        <width>4</width>
      </LineStyle>
      <PolyStyle>
        <color>7f00ff00</color>
      </PolyStyle>
    </Style>
    <Placemark>
      <name>Start</name>
      <description>Test Start Position</description>
      <styleUrl>#yellowLineGreenPoly</styleUrl>
      <LineString>
        <extrude>1</extrude>
        <tessellate>1</tessellate>
        <altitudeMode>absolute</altitudeMode>
        <coordinates>
            %(coordinates)s
        </coordinates>
      </LineString>
    </Placemark>
  </Document>
</kml>
''' % {
    'testname': self.test.name,
    'date': self.test.date,
    'coordinates': ' '.join(['%.9f,%.9f,%d' % c for c in coords])
}
