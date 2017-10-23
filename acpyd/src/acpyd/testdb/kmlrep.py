from zope.interface import implements

from twisted.python import components

from acpyd.testdb import TestDb, TestLogLine
from acpyd.testdb.interfaces import IKMLTestReport

class KMLTestReport(object):
    implements(IKMLTestReport)
    def __init__(self, store):
        self.store = store

    def get_kml(self, test_id):
        test = self.store.get_test(test_id)
        logs = list(self.store.query(
            TestLogLine,
            TestLogLine.test == test
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
    'testname': test.name,
    'date': test.date,
    'coordinates': ' '.join(['%.9f,%.9f,%d' % c for c in coords])
}
def register():
    components.registerAdapter(
            KMLTestReport,
            TestDb,
            IKMLTestReport
    )
