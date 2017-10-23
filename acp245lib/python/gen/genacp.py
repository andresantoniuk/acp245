from Synopsis.process import process
from Synopsis.Processor import Composite
from Synopsis.Parsers import Cpp
from Synopsis.Parsers import C
from Synopsis.Formatters import Dump
from Synopsis.IR import IR
from Synopsis import ASG
import os
import sys
import re
from StringIO import StringIO

# This exists to force some names to names used before writing this generator
FORCED_NAMES = {
    'AlarmKa' : 'AlarmKA',
    'AlarmKaReply' : 'AlarmKAReply',
    'IEGpsRawData' : 'IEGPSRawData',
    'IETcuDesc' : 'IETCUDesc',
    'IETcuData' : 'IETCUData',
    'IETcuDataItem' : 'IETCUDataItem',
    'IETcuDataError' : 'IETCUDataError',
    'IETcuDataErrorItem' : 'IETCUDataErrorItem',
}
# No mappings to Python are provided for these classes
IGNORE_CLASSES = (
    'acp_msg',
    'acp_msg_data',
)
TYPE_MAP = {
    'acp_el_presence': 'int',

    'acp_el_transmit_unit': 'int',
    'acp_app_id': 'int',
    'acp_el_ctrl_entity': 'int',
}
APP_ID_MAP = {
    'ACP_APP_ID_FUNC': 'ACP_APP_ID_REMOTE_VEHICLE_FUNCTION',
    'ACP_APP_ID_TRACK': 'ACP_APP_ID_VEHICLE_TRACKING',
    'ACP_APP_ID_CFG': 'ACP_APP_ID_CONFIGURATION',
    'ACP_APP_ID_PROV': 'ACP_APP_ID_PROVISIONING',
}
MSG_TYPE_MAP = {
    'ACP_MSG_TYPE_CFG_ACTIVATION': 'ACP_MSG_TYPE_CFG_ACT_245',
}

class Util:

    def base_file(self, filename):
        return os.path.basename(filename)

    def get_to_py_conversor(self, tp, var_name):
        if isinstance(tp, Array):
            return '[%s for i in range(%s_cnt)]' % (
                self.get_to_py_conversor(tp.type, '%s[i]' % var_name),
                var_name
            )
        if isinstance(tp, IEAny):
            return 'acp_ie_any_to_py(&%s)' % (var_name)
        elif isinstance(tp, String):
            return 'str_or_none(<char*>%s)' % (var_name)
        elif isinstance(tp, IP):
            return 'u32_ntoa(%s)' % (var_name)
        elif isinstance(tp, Buffer):
            return 'u8_ptr_to_tuple(%s, %s_len)' % (var_name, var_name)
        elif isinstance(tp, List):
            return '[%s for i in range(%s_cnt)]' % (
                self.get_to_py_conversor(tp.type, '%s[i]' % var_name),
                var_name
            )
        elif isinstance(tp, Element):
            return '%s_to_py(&%s)' % (tp.name, var_name)
        elif isinstance(tp, UnionElement):
            return var_name
        else:
            return var_name

    def get_to_c_conversor(self, tp, to_var, var_name):
        if isinstance(tp, Array):
            if isinstance(tp.type, Element):
                return '''
        cdef int cnt = min(len(%(var)s), ACP_EL_LOC_DELTA_MAX)
        %(to_var)s_cnt = cnt
        for i from 0 <= i < cnt:
            %(to_c_conversor)s
        ''' % {
            'to_var': to_var,
            'var': var_name,
            'type': tp.type,
            'to_c_conversor': self.get_to_c_conversor(tp.type,
                                                      '%s[i]' % to_var,
                                                      '%s[i]' % var_name),
            }
            else:
                return '''
        %(to_var)s_cnt = min(len(%(var)s), %(max_array_size)d)
        #%(to_var)s_present = 1
        list_to_u8_ptr_f(%(var)s[:%(to_var)s_cnt], %(to_var)s, %(to_var)s_cnt)
            ''' % {
            'max_array_size': tp.size,
            'to_var': to_var,
            'var': var_name,
            'type': tp.type
            }
        elif isinstance(tp, IEAny):
            return '''
        if %(var)s is not None:
            to_acp_ie_any(&%(to_var)s, %(var)s)
        else:
            %(to_var)s.present = 1
            ''' % {
            'to_var': to_var,
            'var': var_name,
            }
        elif isinstance(tp, String):
            return '''
        if %(var)s is not None:
            __temp = %(var)s
            %(to_var)s = strdup(__temp)
            __temp = None
            ''' % {
            'to_var': to_var,
            'var': var_name,
            }
        elif isinstance(tp, IP):
            c = '(%s is not None) and u32_aton(%s) or 0' % (var_name, var_name)
        elif isinstance(tp, Buffer):
            # assuming all our lengths are no larger than u8...
            return '''
        cdef u32 l
        %(to_var)s = str_tuple_to_u8_ptr(%(var)s, &l)
        if l > %(max_size)d:
            raise ValueError('Is too large')
        %(to_var)s_len = <u%(width)s> l
        ''' % {
            'max_size': 2**tp.len_size,
            'width': tp.len_size,
            'to_var': to_var,
            'var': var_name,
            }

        elif isinstance(tp, List):
            return '''
        l = len(%(var)s)
        if l > %(max_size)d:
            raise ValueError('Has to many items')
        %(to_var)s_cnt = <u%(width)s> l
        %(to_var)s = <c_pdu.%(type)s*>malloc(sizeof(c_pdu.%(type)s) * l)
        memset(%(to_var)s, 0, sizeof(c_pdu.%(type)s) * l)
        for i from 0 <= i < l:
            %(to_c_conversor)s''' % {
            'max_size': 2**tp.len_size,
            'width': tp.len_size or 32,
            'to_var': to_var,
            'var': var_name,
            'type': tp.type,
            'to_c_conversor': self.get_to_c_conversor(tp.type,
                                                      '%s[i]' % to_var,
                                                      '%s[i]' % var_name),
            }
        elif isinstance(tp, Element):
            return '(<__c__%s>%s)._to_struct(&%s)' % (self.to_cython_name(tp),
                                                 var_name, to_var)
        elif isinstance(tp, UnionElement):
            c = 'object'
        elif tp in TYPE_MAP:
            c = '<c_pdu.%s>(<%s>%s)' % (tp,TYPE_MAP[tp], var_name)
        else:
            c = var_name
        return '%s = %s' % (to_var, c)

    def to_cython_name(self, el):
        name = el.name
        if name.startswith('acp_el'):
            new_name = 'IE' + ''.join(x.capitalize() for x in name.split('_')[2:])
        elif name.startswith('acp_msg'):
            new_name = ''.join(x.capitalize() for x in name.split('_')[2:])
        else:
            raise Exception('Unknown name: %s' % name)

        return FORCED_NAMES.get(new_name, new_name)

    def to_c_app_id_constant(self, msg):
        app_id = 'ACP_APP_ID_%s' % msg.name.split('_')[2].upper()
        return APP_ID_MAP.get(app_id, app_id)

    def to_c_msg_type_id_constant(self, msg):
        msg_type = 'ACP_MSG_TYPE_%s' % '_'.join(msg.name.split('_')[2:]).upper()
        return MSG_TYPE_MAP.get(msg_type, msg_type)

    def get_initializer(self, fld):
        if isinstance(fld.type, List) or isinstance(fld.type, Array):
            return '''
        if %(var)s is None:
            self.%(var)s = []
        else:
            self.%(var)s = %(var)s
            ''' % { 'var': fld.name }
        else:
            return 'self.%(var)s = %(var)s' % { 'var': fld.name }

    def get_default_value(self, fld):
        if isinstance(fld.type, str):
            return '0'
        else:
            return 'None'

    def get_cython_type(self, tp):
        if isinstance(tp, Array):
            return 'object'
        elif isinstance(tp, IEAny):
            return 'object'
        elif isinstance(tp, List):
            return 'object'
        elif isinstance(tp, String):
            return 'object'
        elif isinstance(tp, IP):
            return 'object'
        elif isinstance(tp, Buffer):
            return 'object'
        elif isinstance(tp, UnionElement):
            return 'object'
        elif isinstance(tp, Element):
            return self.to_cython_name(tp)
        else:
            return TYPE_MAP.get(tp.replace(' ',''), tp)

    def is_element(self, tp):
        return isinstance(tp, Element)

    def is_primitive(self, tp):
        return isinstance(tp, str)

    def is_any(self, fld):
        return isinstance(fld, IEAny)

    def has_present(self, fld):
        if isinstance(fld, Element):
            return fld.has_present()
        return False

class CVariable:
    def __init__(self, name, type, annotations):
        self.name = name

        arr_idx = type.find('[')
        if arr_idx != -1:
            self.dim = type[arr_idx+1:-1]
            self.type = type[:arr_idx]
        else:
            self.dim = ''
            self.type = type
        self.annotations = annotations

class Field:
    def __init__(self, type, name, toggles=None):
        self.type = type
        self.name = name
        if toggles is None:
            self.toggles = None
        else:
            self.toggles = toggles
        self.toggled = False

    def __str__(self):
        return self.name

class UnionElement:
    def __init__(self, name):
        self.name = name
        self.fields = []
        self.c_fields = []

class Element:
    def __init__(self, name):
        self.name = name
        self.fields = []
        self.c_fields = []

    def get_field(self, name):
        for fld in self.fields:
            if fld.name == name:
                return fld
        return None

    def has_required_fields_after(self, fld):
        return [x for x in self.fields[self.fields.index(fld)+1:] if \
                (not isinstance(x.type, Element) or not x.type.has_present())]

    def get_optional_fields_after(self, fld):
        return [x for x in self.fields[self.fields.index(fld)+1:] if \
                (isinstance(x.type, Element) and x.type.has_present())]

    def has_present(self):
        for fld in self.fields:
            if fld.name == 'present' and fld.type == 'acp_el_presence':
                return True
        return False

    def get_toggle_fields(self):
        return [x for x in self.fields if x.toggles]

    def __str__(self):
        return self.name

class String:
    def __init__(self, c_variable):
        self.c_variable = c_variable

class IP:
    def __init__(self, c_variable):
        self.c_variable = c_variable

class Buffer:
    def __init__(self, len_size, c_variable):
        self.len_size = len_size
        self.c_variable = c_variable

class IEAny:
    def __init__(self, c_variable):
        self.c_variable = c_variable

class Array:
    def __init__(self, type, size, len_size, c_variable):
        self.type = type
        self.size = size
        self.len_size = len_size
        self.c_variable = c_variable

class List:
    def __init__(self, type, len_size, c_variable):
        self.type = type
        self.len_size = len_size
        self.c_variable = c_variable

class Enum:
    def __init__(self, name):
        self.name = name
        self.values = []

class Message:
    def __init__(self, name):
        self.name = name
        self.fields = []
        self.c_fields = []

    def get_field(self, name):
        for fld in self.fields:
            if fld.name == name:
                return fld
        raise KeyError('No such field: %s' % name)

    def has_required_fields_after(self, fld):
        return [x for x in self.fields[self.fields.index(fld)+1:] if \
                (not isinstance(x.type, Element) or not x.type.has_present())]

    def get_optional_fields_after(self, fld):
        return [x for x in self.fields[self.fields.index(fld)+1:] if \
                (isinstance(x.type, Element) and x.type.has_present())]

    def get_toggle_fields(self):
        return [x for x in self.fields if x.toggles]

class Function:
    def __init__(self, return_type, name, parameters):
        self.return_type = return_type
        self.name = name
        self.parameters = parameters

class ClassDef:
    def __init__(self, name, type):
        self.name = name
        self.fields = []
        self.c_fields = []
        self.type = type

    def get_field(self, name):
        for fld in self.fields:
            if fld.name == name:
                return fld
        return None

class Protocol:
    def __init__(self):
        self.messages = []
        self.elements = []
        self.enums = []
        self.constants = []
        self.functions = []
        self.classes = []

class Toggle:
    def __init__(self, field, flag):
        self.field = field
        self.flag = flag

ANN_TYPE_RE = re.compile('@type\s+([^ ]+)', re.MULTILINE)
ANN_TOGGLES_RE = re.compile('@toggles\s+([^, ]+),([^ ]+)', re.MULTILINE)
ANN_FORCE_INCLUDE_RE = re.compile('@force_include', re.MULTILINE)
class CToCythonFieldVisitor(ASG.Visitor):
    def __init__(self, item, elements):
        self.item = item
        self.elements = elements
        self.len_size = {}

    def _get_type(self, name, type, annotations):
        c_variable = CVariable(name, type, annotations)
        # check if it's an array
        arr_idx = type.find('[')
        if arr_idx != -1:
            arr_type = self._get_type(name, type[:arr_idx], annotations)
            arr_len = int(type[arr_idx+1:type.find(']')])
            type = Array(arr_type, arr_len, self.len_size[name], c_variable)
        # check if it's a pointer
        elif type.find('*') != -1:
            type = type.replace('*','')
            if type in ('ascii', 'char'):
                type = String(c_variable)
            elif type in ('u8'):
                type = Buffer(self.len_size.get(name), c_variable)
            else:
                list_type = self._get_type(name, type.replace('*',''),
                                           annotations)
                type = List(list_type, self.len_size.get(name), c_variable)
        elif type == 'acp_ie_any':
            type = IEAny(c_variable)
        elif type == 'IP':
            type = IP(c_variable)
        elif annotations:
            m = ANN_TYPE_RE.search(''.join(annotations.get('comments',[])))
            if m:
                vtype = m.group(1)
                if vtype == 'IP':
                    if type != 'u32':
                        raise ValueError('IP can only be u32')
                    return IP(c_variable)
                else:
                    raise ValueError('Unknown virtual type')
        return self.elements.get(type, type)

    def visit_variable(self, node):
        name = str(node.name).split(node.name.sep)[-1]
        type = str(node.vtype)
        self.item.c_fields.append(CVariable(name, type, node.annotations))

        for ending in ('_cnt', '_len'):
            if name.endswith(ending):
                try:
                    name = name[:-len(ending)]
                    size = int(type.replace('u',''))
                    self.len_size[name] = size
                except ValueError:
                    raise Exception('%s element must be a type of unsigned int and used to count on an array' % name)
                # list or array counter, will be considered when the array is found
                return

        toggles = ANN_TOGGLES_RE.search(''.join(node.annotations.get('comments',[])))
        if toggles:
            toggle_field, toggle_flag = toggles.groups()
            toggles = Toggle(self.item.get_field(toggle_field), toggle_flag)
            toggles.field.toggled = True
        self.item.fields.append(
            Field(
                self._get_type(name, type, node.annotations),
                name,
                toggles
            )
        )

class CStructToCythonClassVisitor(ASG.Visitor):
    def start(self, ir):
        self.elements = {}

        self.protocol = Protocol()
        for decl in ir.asg.declarations:
            decl.accept(self)
        return self.protocol

    def visit_function(self, node):
        return_type = str(node.return_type)
        name = str(node.real_name)
        parameters = [self.__strip_qual(str(param.type), 'const') + ' ' + \
                      str(param.name) for param in node.parameters]
        self.protocol.functions.append(Function(return_type, name, parameters))

    def visit_macro(self, node):
        name = str(node.name)
        # ignore macro functions...
        if name.startswith('ACP') and not node.parameters:
            if node.text.startswith('"') or node.text.startswith("'"):
                self.protocol.constants.append((name, str))
            else:
                self.protocol.constants.append((name, int))

    def visit_enum(self, node):
        enum = Enum(str(node.name))
        for child in node.enumerators:
            enum.values.append(child.name)
        self.protocol.enums.append(enum)

    def visit_class(self, node):
        item = None
        name = str(node.name)

        # always record a union or struct
        item = ClassDef(name, node.type)
        self.protocol.classes.append(item)
        v = CToCythonFieldVisitor(item, self.elements)
        for child in node.declarations:
            child.accept(v)

        # see if it's a protocol class
        if name in IGNORE_CLASSES:
            return
        item = None
        # no union representation in python
        if node.type == 'union':
            item = UnionElement(name)
            self.elements[name] = item
        if name.startswith('acp_el'):
            item = Element(name)
            self.elements[name] = item
            self.protocol.elements.append(item)
        elif name.startswith('acp_msg'):
            item = Message(name)
            self.protocol.messages.append(item)

        if item:
            v = CToCythonFieldVisitor(item, self.elements)
            for child in node.declarations:
                child.accept(v)

    def __strip_qual(self, name, qual):
        s = name.find(qual + ' ')
        if s != -1:
            return (name[:s] + name[s + len(qual):]).strip()
        else:
            return name

from Cheetah.Template import Template

def apply_template(template_file, protocol):
    tmpl = Template.compile(file=template_file)
    return str(
        tmpl(
            namespaces= {
                'proto':protocol,
                'util': Util()
            }
        )
    )

def parse_protocol(input_files, cpp_args):
    parse = C.Parser(base_path = '.' + os.sep, cppflags=cpp_args)
    ir = IR()
    parse.process(ir, input=input_files)
    protocol = CStructToCythonClassVisitor().start(ir)
    protocol.file = input_files
    return protocol
