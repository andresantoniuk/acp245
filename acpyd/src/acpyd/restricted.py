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
Restricted environment for execution of test scripts.
"""
import types

from acpyd.interfaces import IAutoReplyManager
import acp245.pdu

from acpyd.RestrictedPython.RCompile import RestrictedCompileMode, RestrictedModuleCodeGenerator
from acpyd.RestrictedPython.RestrictionMutator import RestrictionMutator
from compiler.pycodegen import Module, ModuleCodeGenerator
from acpyd.RestrictedPython.Guards import safe_builtins

__all__ = ['restricted_exec']

class BenchRestrictionMutator(RestrictionMutator):
    """Mutator that modifies the handling of Python constructs. It extends
    RestrictedPython RestrictionMutator by changing it behavior as follows.
    By defaults it allows:
        * yield
        * increment assign (+=)
    And disallows:
        * class definitions
        * module imports
    """
    def visitYield(self, node, walker):
        """Visits a yield statement."""
        return walker.defaultVisitNode(node)
    def visitClass(self, node, walker):
        """Visits a class def statement."""
        self.error(node, 'Class definition is not allowed.')
    def visitImport(self, node, walker):
        """Visits an import statement."""
        self.error(node, 'import statement is not allowed.')
    def visitAugAssign(self, node, walker):
        """Visits an augment assign (+=) statement."""
        return walker.defaultVisitNode(node)

class BenchRestrictedCompileMode(RestrictedCompileMode):
    """A compilation mode that uses the BenchRestrictionMutator."""
    def __init__(self, *args):
        RestrictedCompileMode.__init__(self, *args)
        self.rm = BenchRestrictionMutator()

class BenchRModule(BenchRestrictedCompileMode, Module):
    """A module compilation that uses the RestrictedModuleCodeGenerator"""
    mode = "exec"
    CodeGeneratorClass = RestrictedModuleCodeGenerator

class AccessDenied(Exception):
    """Raised when code tries to use a restricted Python feature."""

def restricted_exec(bench, source, filename, globals):
    """Execs the given source, read from the given filename, using the given
    globals. The code will be executed as a BenchRModule, therefore restricting
    it's behavior.

    Print statements will send output to the bench info logger,
    by calling bench.info(text).
    """
    gen = BenchRModule(str(source), str(filename))
    gen.compile()
    class r_PrintHandler:
        """A handler of print statements."""
        def write(self, text):
            """Called when print is interpreted."""
            bench.info(text)
    def r_write(o):
        """Called when a write is being performed on object o."""
        if (type(o) in (
            types.InstanceType,
            types.ListType,
            types.DictType,
            types.TupleType,
            types.StringType,
            types.FloatType,
            types.IntType,
            types.LongType,
        )
            or isinstance(o, acp245.pdu.Element)
            or isinstance(o, acp245.pdu.Message)
            or isinstance(o, acp245.pdu.Header)
            or IAutoReplyManager.providedBy(o)
        ):
            return o
        else:
            raise AccessDenied('write not allowed to objects of type %s' % type(o))
    def r_getitem(ob, index):
        """Called when accessing an item (__getitem__) on object ob."""
        # No restrictions.
        return ob[index]
    def r_getiter(ob):
        """Called when accessing an iterator iter(ob) on object ob."""
        return iter(ob)

    # remap python builtins
    builtins = dict(safe_builtins)
    builtins['hasattr'] = hasattr
    globals['__builtins__'] = builtins
    globals['_print_'] = r_PrintHandler
    globals['_write_'] = r_write
    globals['_getitem_'] = r_getitem
    globals['_getiter_'] = r_getiter
    globals['_getattr_'] = getattr

    # execute the code with the given globals.
    exec gen.getCode() in globals
