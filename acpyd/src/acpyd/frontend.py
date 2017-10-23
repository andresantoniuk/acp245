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
"""A simple module to define a page on which childrens can be dynamically
added.
"""
# At this time the page template (frontend.html) lists the children buttons
# statically, so this module is not really general at all. The page
# should be able to render the child buttons based on the registered children
# to make this class more reusable.
from acpyd.web import BasePage
from nevow import rend, inevow

class FrontendPage(BasePage):
    """Create a new frontend page."""
    template = 'frontend.html'

    def __init__(self, log, authorizator=None):
        """Creates a page that logs new requests to log and uses the given
        authorizator to control access to the page."""
        BasePage.__init__(self, authorizator=authorizator)
        self.childs = {}
        self.log = log

    def add_child(self, name, page):
        """Adds a new children for the given path name."""
        self.childs[name] = page

    def childFactory(self, ctx, name):
        """Returns the selected children."""
        request = inevow.IRequest(ctx)
        if self.log:
            self.log.debug('requested %s', request.uri)
        if name in self.childs:
            if request.path.endswith(name):
                request.redirect(request.uri + '/')
                return ''
            else:
                return self.childs[name]()
        elif name == '':
            return FrontendPage(self.log, authorizator=self.authorizator)
        else:
            return rend.FourOhFour()
