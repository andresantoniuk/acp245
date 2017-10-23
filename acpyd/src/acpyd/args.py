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
"""
Message argument parsing routines.

These functions are used to receive arguments (for example through HTTP) and
inject them into an ACP245 PDU, and to extract arguments from an ACP245 PDU to
be sent as a response or to call another function that receives parameters for
an ACP245 PDU.

The idea is to have a common definition of the message fields using the provided
descriptors, and then map them to custom argument names. The common field
definitions are usually provided in the file flatten.py.

To use the arguments->message function, you need to provide a set of descriptors
(instances of Int, Int8, String, Time, etc.) that describe the arguments, use
them to parse a dictionary of {<key>, <value>} that has the argument value for
each message field name, and provide a message where to inject the parsed
arguments.

To use the message->argument, you only need to provide the descriptors and the
message, and the function will return a value dictionary with the arguments.

Example:
>>> from acp245.pdu import CfgUpd245
>>> descriptors = (
... ('my_argument_1', Int8('header.version')),
... ('my_iccid', String('vehicle_desc.iccid')),
... )
>>> m = CfgUpd245()
>>> insert_args_in_msg(descriptors, {'my_argument_1': 10, 'my_iccid': '1234'}, m)
>>> m.header.version
10L
>>> m.vehicle_desc.iccid
'1234'
>>> insert_msg_in_args(descriptors, m)
{'my_argument_1': 10, 'my_iccid': '1234'}

To perform the injection, the functions will try to infer complex types from the
field name. For example, to inject the ICCID into the Vehicle Descriptor, is
first necessary to create the IEVehicleDesc and then set the ICCID on the
created element. By convention, field names have the same name as their data
type (for example, vehicle_desc is a IEVehicleDesc, ctrl_func is a IECtrlFunc,
version is a IEVersion, etc.). If it's not possible to infer the element type
from the field name, it's necessary to provide a hint, as in the following case:
    ('errors', List('error(IETCUDataError).items', IETCUDataErrorItem, (
                        ('type', Int8('type')),
                        ('data', BinHex('data')),
                        ('code', Int8('error.code'))
                        )
                    )
    )
Where we are saying that the argument called 'errors' is mapped to a field
called error.items, where the type of the error field is IETCUDataError, and the
type of the argument value is a list. So when the function is injecting the
'errors' argument into the message, it first will create a IETCUDataError
element, assign it to the 'error' field of the message, and then assigns the
'items' field of the created element to the value of the 'errors' argument.
"""

__all__ = [
    "FormatError", "RequiredError",
    "insert_args_in_msg", "insert_msg_in_args",
    "Int", "Int8", "Int16", "Int32", "String", "Time", "BinHex", "List"
]

import binascii
import datetime
import calendar
from acp245 import pdu

class FormatError(Exception):
    """Raised when an argument format is invalid."""

class RequiredError(Exception):
    """Raised when a required argument is not provided."""

def __parse_descriptors(descriptors):
    """
    Parses the descriptors in tuple format returning a
    {key: fld} format.

    Tuple format is a tuple of descriptor tuples, where each descriptor tuple
    can be one of:
    ('fld_1', Int8('header.version')),  # Instance of a descriptor.
    ('fld_2', Int8, 'header.version', {required=True}), # Descriptor class
                                                        # followed by
                                                        # keyword arguments
    ('fld_3', Int8, 'header.version'),  # Descriptor class
    """
    argsflds = {}
    for (key, desc) in descriptors:
        if isinstance(desc, ArgType):
            fld = desc
        elif len(desc) == 3:
            fld = desc[0](desc[1], **desc[2])
        else:
            fld = desc[0](desc[1])
        argsflds[key] = fld
    return argsflds

def insert_args_in_msg(descriptors, args, msg):
    """Inserts the given arguments into the given message, according to the
    descriptors.
    Returns the message.
    """
    for key, desc in __parse_descriptors(descriptors).items():
        desc.insert(key, args, msg)
    return msg

def insert_msg_in_args(descriptors, msg):
    """Inserts the message fields in an arguments dictionary, according to the
    descriptors.
    Returns the argument dictionary."""
    args = {}
    if isinstance(msg, pdu.Message) or isinstance(msg, pdu.Element):
        for key, desc in __parse_descriptors(descriptors).items():
            desc.extract(key, args, msg)
    elif isinstance(msg, dict):
        # nothing special to do
        args.update(msg)
    else:
        raise Exception('Unsupported msg type: %s' % type(msg) )
    return args

class ArgType(object):
    """A base class for argument descriptors."""
    def __init__(self, note=None):
        """Note is a comment to describe the argument. Used for automatic
        documentation generation."""
        self.note = note

    def extract(self, msg):
        """Extracts the value of the message field according to this
        descriptor.
        Raises FormatError if the argument format is invalid.
        """
        raise NotImplemented

    def insert(self, field, values, msg):
        """Inspect values for the given field value and injects its into
        message.
        Values is a dictionary of keys -> values.
        One descriptor may use a one or more keys to get the field value that
        will be injected into the message, as per the descriptor documentation.
        Raises FormatError if the argument format is invalid.
        """
        raise NotImplemented

    def _parse_name(self, name):
        """
        Returns the actual field name from the descriptor field name. The
        descriptor field name may include additional information to be able to
        inject the argument into the message, for example the name may be
        error(TCUDataError) to indicate that the field name is error, but the
        actual field value type is TCUDataError.
        """
        if '(' in name:
            return (name[:name.index('(')],
                   name[name.index('(')+1:-1])
        else:
            return name, None

    def _get_member(self, msg_field, msg):
        """
        Returns the member called msg_field from message.
        msg_field has the following format:
            <element>.[<sub_element>...].<field_name>
        For example:
            vehicle_desc.iccid

        Also, each element may be annotated with its type, like for example:
            location.curr_gps(IEGPSRawData).lat
        Where we say that the curr_gps field is of type GPSRawData.
        """
        path = msg_field.split('.')
        member = msg
        for p in path[:-1]:
            name, type_name = self._parse_name(p)
            submember = getattr(member, name)
            if submember is None:
                return None
            member = submember
        name, type_name = self._parse_name(path[-1])
        v = getattr(member, name)
        return v

    def _set_member(self, value, msg_field, msg):
        """
        Sets the member called msg_field in message.
        The format of the msg_field is described in the _get_member function and
        the module documentation.
        """
        path = msg_field.split('.')
        member = msg
        for p in path[:-1]:
            name, type_name = self._parse_name(p)
            submember = getattr(member, name)
            if submember is None:
                if type_name:
                    cls = getattr(pdu, type_name)
                else:
                    # FIXME: IETCU* classes do not follow convention. Changing breaks
                    # backward compat.
                    ie_name = 'IE%s' % ''.join([(x != 'tcu' and x.capitalize() or
                                                'TCU') for x in p.split('_')])
                    cls = getattr(pdu, ie_name, None)
                if cls is not None:
                    submember = cls()
                    setattr(member, name, submember)
            member = submember

        name, typ = self._parse_name(path[-1])
        setattr(member, name, value)

class List(ArgType):
    """A List descriptor.
    Describes an argument whose value is a list.
    The values are provided in the following format, assuming the argument is
    called 'my_arg':
        {
            'my_arg_cnt': 2,
            'my_arg_1': 10,
            'my_arg_2': 20,
        }

    For a list of two elements, 10 and 20. In this case the descriptor could
    be given as:
        ('my_arg', List('<msg field name>', int))

    Where <msg field name> is the name of the field to set in the message and
    int the type of the list element.

    Also, if the list is actually a list of elements, the value could be given
    as:
        {
            'my_arg_cnt': 2,
            'my_arg_1_cod': 1,
            'my_arg_1_val': 'ff',
            'my_arg_2_cod': 2,
            'my_arg_2_val': 'ab',
        }

    In this case, the descriptor could be given as:
        ('my_arg', List('tcu_data.items', IETCUDataItem, (
            ('cod': Int8('type')),
            ('val': BinHex('data')),
        )))

    Where tcu_data.items is the message field name, IETCUDataItem the type of
    the list elements, and 'cod' and 'val' descriptors to inject the cod and val
    arguments into the list elements.

    """
    def __init__(self, msg_field, item_type, descriptors=None, note=None):
        """Create a new list descriptor.
        msg_field is the name of the message field.
        item_type is the type of the list items.
        descriptors is an optional descriptor tuple for when the item_type is an
        element with sub fields.
        """
        super(List, self).__init__(note=note)
        self.msg_field = msg_field
        self.item_type = item_type
        self.descriptors = descriptors

    def extract(self, key, args, msg):
        """Extracts the value of the message field according to this
        descriptor."""
        try:
            l = self._get_member(self.msg_field, msg)
        except AttributeError:
            return

        if l is None:
            l = []
        if self.descriptors is None:
            args[key] = ','.join([str(x) for x in l])
        else:
            cnt = len(l)
            args['%s_cnt' % key] = cnt
            for i in range(cnt):
                prefix = '%s_%d_' % (key, i)
                subargs = insert_msg_in_args(self.descriptors, l[i])
                prefixed_subargs = dict(('%s%s' % (prefix, k), v) for k, v in
                                        subargs.items())
                args.update(prefixed_subargs)

    def insert(self, key, values, msg):
        """Inspect values for the given field value and injects its into
        message."""
        l = []
        if self.descriptors is None:
            if key in values:
                l = [self.item_type(x) for x in values[key].split(',') if x]
        else:
            cnt = int(values.get('%s_cnt' % key, 0))
            for i in range(cnt):
                prefix = '%s_%d_' % (key, i)
                subargs = dict([(x[len(prefix):],y) for x,y in values.items() if x.startswith(prefix)])
                item = self.item_type()
                insert_args_in_msg(self.descriptors, subargs, item)
                l.append(item)
        self._set_member(l, self.msg_field, msg)

class SingleArg(ArgType):
    """Base class for descriptors on which the argument is a single value."""
    def __init__(self, msg_field, required=False, note=None):
        """Creates a new argument that accepts a single value."""
        super(SingleArg, self).__init__(note=note)
        self.msg_field = msg_field
        self.required = required

    def validate(self, value):
        """Validates the argument values.
        Returns True if valid, False otherwise."""
        try:
            self._parse(value)
            return True
        except FormatError:
            return False

    def extract(self, key, args, msg):
        """Extracts the value of the message field according to this
        descriptor."""
        try:
            v = self._get_member(self.msg_field, msg)
            if v is not None:
                uv = self._unparse(v)
                if uv is not None:
                    args[key] = str(uv)
        except AttributeError:
            pass

    def insert(self, key, values, msg):
        """Inspect values for the given field value and injects its into
        message."""
        if key in values:
            return self._set_member(self._parse(values[key]), self.msg_field, msg)
        elif self.required:
            raise RequiredError('%s is required' % key)

    def _parse(self, value):
        """Subclasses must override this method to parse a string value to a
        argument value."""
        raise NotImplementedError()

    def _unparse(self, value):
        """Subclasses must override this method to parse a value to a string
        to store on the argument dictonary."""
        raise NotImplementedError()

class Int(SingleArg):
    """An integer."""

    def __init__(self, msg_fields, required=False, values=None,
                 min=None, max=None, note=None):
        """Creates a new integer.

        values can be a dictionary from 'names' to values which are used to
        parse the argument. For example:

            ('my_arg': Int('header.msg_ctrl',
                values={
                    'RESP_EXP': 1,
                }))
        Will accept the following *equivalent* values in the argument values
        dictionary:
            {'my_arg': 1} or
            {'my_arg': 'RESP_EXP'} or
            {'my_arg': '0x1'} or
            {'my_arg': '1'}
        """
        assert (values is None or
                (isinstance(values, list) or
                 isinstance(values, dict)))
        super(Int, self).__init__(msg_fields, required=required, note=note)
        self.values = values
        self.min = min
        self.max = max

    def _parse(self, value):
        """Parses the given value, raising an exception is the format is
        invalid.
        Raises FormatError if the format is invalid.
        """
        if value is None:
            if self.required:
                raise FormatError('Value is required')
            else:
                return None
        try:
            # check if value is int
            if (isinstance(value, basestring) and
                (value.startswith('0x') or value.startswith('0X'))):
                i = int(value, 16)
            else:
                i = int(value)
        except:
            if self.values is not None and isinstance(self.values, dict):
                i = self.values.get(value)
                if i is None:
                    raise FormatError('Value is not an allowed option')
            else:
                raise FormatError('Not an integer')
        else:
            if self.values is not None:
                if isinstance(self.values, list):
                    if i not in self.values:
                        raise FormatError('Value is not an alowed option')

        if (self.min is not None and i < self.min):
            raise FormatError('Value must be greater than %d' % self.min)
        if (self.max is not None and i > self.max):
            raise FormatError('Value must be less than %d' % self.min)
        return i

    def _unparse(self, value):
        """Transforms the int value to an int or to a constant name is 'values'
        was provided when constructed."""
        if self.values is not None and isinstance(self.values, dict):
            for key, val in self.values.items():
                if val == value:
                    return key
        return value

class Int8(Int):
    """An 1 byte integer."""
    def __init__(self, msg_fields, required=False, values=None, note=None):
        super(Int8, self).__init__(msg_fields, required=required,
                                   values=values,
                                   min=0, max=0xFF, note=note)

class Int16(Int):
    """An 2 bytes integer."""
    def __init__(self, msg_fields, required=False, values=None, note=None):
        super(Int16, self).__init__(msg_fields, required=required,
                                   values=values,
                                   min=0, max=0xFFFF, note=note)

class Int32(Int):
    """An 4 bytes integer."""
    def __init__(self, msg_fields, required=False, values=None, note=None):
        super(Int32, self).__init__(msg_fields, required=required,
                                   values=values,
                                   min=0, max=0xFFFFFFFF, note=note)

class Time(SingleArg):
    """
    A date/time value.
    Format is "%Y-%m-%d %H:%M:%S".
    The argument is always mapped to a acp245.pdu.IETimestamp element.
    """
    def _parse(self, value):
        """Parses the value to a timestamp element."""
        try:
            # check if value is int
            ts = float(value)
        except:
            if value is None or value == '':
                return None
            try:
                dt = datetime.datetime.strptime(value, "%Y-%m-%d %H:%M:%S")
                return pdu.IETimestamp(
                    year = dt.year,
                    month = dt.month,
                    day = dt.day,
                    hour = dt.hour,
                    minute = dt.minute,
                    second = dt.second)
            except:
                raise FormatError('Invalid date time format')
        else:
            return pdu.IETimestamp(time=ts)

    def _unparse(self, value):
        """Transforms a timestamp element value into a string representation."""
        if value is None or not value.month:
            return None
        return calendar.timegm(
                (value.year, value.month, value.day,
                 value.hour, value.minute, value.second))

class BinHex(SingleArg):
    """A hexadecimal string that represents binary data."""
    def _parse(self, value):
        """Parses the hex string and returns the binary data."""
        try:
            b = binascii.a2b_hex(value)
            return b
        except:
            raise FormatError('Invalid hex string')

    def _unparse(self, value):
        """Transforms the binary data into a hex string."""
        if isinstance(value, basestring):
            return binascii.b2a_hex(value).upper()
        elif isinstance(value, tuple) or isinstance(value, list):
            return binascii.b2a_hex(''.join(map(chr,value))).upper()

class String(SingleArg):
    """A String argument."""
    def _parse(self, value):
        """Tries to parse the value as a string."""
        try:
            return str(value)
        except:
            raise FormatError('Invalid string')

    def _unparse(self, value):
        """Returns str(value)."""
        return str(value)
