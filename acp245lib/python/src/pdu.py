"""PDU message classes and processing functions."""
import calendar
import pdu_gen
from time import gmtime
from pdu_gen import *

NOT_PRESENT = object()

# Here we add behavior and extensions to generated classes. To do so, we must
# use a hackish mechanism.
# We need to 'rebind' the classes of the pdu_gen module, so instead of using the
# original generated module classes, it uses the extension classes defined here.
# For that, we store the original class in <classname>_Parent, and then assign
# the new extension class <classname> to pdu_gen.<classname>, so that the class
# in pdu_gen points to the class defined here.

# remember original class
IETimestamp_Parent = pdu_gen.IETimestamp
# define extension class
class IETimestamp(IETimestamp_Parent):
    def __init__(self, *args, **kwargs):
        time = kwargs.get('time', NOT_PRESENT)
        if time is not NOT_PRESENT:
            del kwargs['time']

        if kwargs.setdefault('year', 1990) < 1990:
            raise InvalidArgumentError('year must be greater or equal to 1990')

        # call original class constructor
        IETimestamp_Parent.__init__(self, *args, **kwargs)
        if time is not NOT_PRESENT:
            self.year, self.month, self.day, \
            self.hour, self.minute, self.second = gmtime(time)[:6]
    def get_time(self):
        return calendar.timegm((
            self.year, self.month, self.day,
            self.hour, self.minute, self.second
        ))
# rebind pdu_gen class so for now one when pdu_gen refers to IETimestamp it's
# actually using this module IETimetamp. The only remaining reference to the
# original class should be IETimestamp_Parent
pdu_gen.IETimestamp = IETimestamp

# remember original
IEGPSRawData_Parent = pdu_gen.IEGPSRawData
class IEGPSRawData(IEGPSRawData_Parent):
    def __init__(self, *args, **kwargs):
        coords = kwargs.get('coords', NOT_PRESENT)
        if coords is not NOT_PRESENT:
            del kwargs['coords']

        IEGPSRawData_Parent.__init__(self, *args, **kwargs)

        # If satellites are provided, assume the
        # satellites_avail is the length of the satellites id list
        satellites = kwargs.get('satellites', NOT_PRESENT)
        if satellites is not NOT_PRESENT:
            self.satellites = satellites
        else:
            self.satellites = tuple()

        satellites_avail = kwargs.get('satellites_avail', NOT_PRESENT)
        if satellites_avail is not NOT_PRESENT:
            self.satellites_avail = satellites_avail
        else:
            self.satellites_avail = len(self.satellites)

        if coords is not NOT_PRESENT:
            if not isinstance(coords, tuple) and not isinstance(coords, list):
                raise ValueError('coords must be tuple or list of lat, '\
                                 'lon in degrees')
            if self.area_type == ACP_LOCATION_POINT_1_MILLIARC:
                self.lat = coords[0] * 1000 * 60 * 60
                self.lon = coords[1] * 1000 * 60 * 60
            elif self.area_type == ACP_LOCATION_POINT_100_MILLIARC:
                self.lat = coords[0] * 10 * 60 * 60
                self.lon = coords[1] * 10 * 60 * 60
            else:
                raise ValueError("Invalid area type")

    def _get_coords_in_milliarcsec(self):
        if self.area_type == ACP_LOCATION_POINT_1_MILLIARC:
            return (self.lat, self.lon)
        elif self.area_type == ACP_LOCATION_POINT_100_MILLIARC:
            return (self.lat * 100, self.lon * 100)
        else:
            raise ValueError('Unknown area type units')
    def _set_coords_in_milliarcsec(self, coords):
        if self.area_type == ACP_LOCATION_POINT_1_MILLIARC:
            self.lat = coords[0]
            self.lon = coords[1]
        elif self.area_type == ACP_LOCATION_POINT_100_MILLIARC:
            self.lat = coords[0]/100
            self.lon = coords[1]/100
        else:
            raise ValueError('Unknown area type units')
    coords_in_milliarcsec = property(
        _get_coords_in_milliarcsec,
        _set_coords_in_milliarcsec
    )

    def _set_coords(self, coords):
        if self.area_type == ACP_LOCATION_POINT_1_MILLIARC:
            self.lat = coords[0] * 1000 * 60 * 60
            self.lon = coords[1] * 1000 * 60 * 60
        elif self.area_type == ACP_LOCATION_POINT_100_MILLIARC:
            self.lat = coords[0] * 10 * 60 * 60
            self.lon = coords[1] * 10 * 60 * 60
        else:
            raise ValueError("Invalid area type")
    def _get_coords(self):
        if self.area_type == ACP_LOCATION_POINT_1_MILLIARC:
            return (self.lat / (1000.0 * 60.0 * 60.0), self.lon / (1000.0 * 60.0 * 60.0))
        elif self.area_type == ACP_LOCATION_POINT_100_MILLIARC:
            return (self.lat / (10.0 * 60.0 * 60.0), self.lon / (10.0 * 60.0 * 60.0))
        else:
            raise ValueError('Unknown area type units')
    coords = property(_get_coords, _set_coords)

    def _set_satellites(self, satellites):
        if satellites is None:
            self.satellites_avail = 0
            self.satellites = tuple()
            return
        if not isinstance(satellites, list) and \
           not isinstance(satellites, tuple):
            raise ValueError(
                'satellites must be a list or tuple of satellites IDs')
        self._satellites = tuple(satellites)
        self.satellites_avail = len(self.satellites)
    def _get_satellites(self):
        return self._satellites
    satellites = property(_get_satellites, _set_satellites)
# rebind
pdu_gen.IEGPSRawData = IEGPSRawData
