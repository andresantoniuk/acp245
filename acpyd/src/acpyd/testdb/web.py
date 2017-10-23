import simplejson
import time

import fnmatch
import os

from nevow import rend, inevow, page, loaders, static, athena
from axiom import attributes
from epsilon import extime
from twisted.web import http
from twisted.internet import defer

from acpyd import config
from acpyd.web import BasePage
from acpyd.testdb import TestRun, NoSuchObjectError
from acpyd.testdb.interfaces import IKMLTestReport, IPDFTestReport
from acpyd.interfaces import Observer, Observable

from acpyd.web import (
    LiveSelect,
    ServiceControlButton,
    ServiceStatusLabel,
    TestbenchLog,
    BasePage,
    BaseElement
)
def testdb_list_json(request, db):
    data = {}
    size = int(request.args.get('rows',[30])[0])
    page = int(request.args.get('page',[0])[0])
    search = request.args.get('_search', ['false'])[0]
    result = None
    if search == 'true':
        #sord = request.args.get('sord', ['asc'])[0]
        searchField = request.args.get('searchField', [None])[0]
        searchData = request.args.get('searchString',[''])[0]
        searchOper = request.args.get('searchOper', ['eq'])[0]
        # FIXME, make more general, too complicated if new json requests need
        # to be processed
        if searchField and searchOper:
            if searchField == 'date':
                field = TestRun.date
                try:
                    searchData = extime.Time.fromStructTime(
                        time.strptime(
                            searchData + ' 23:59:59' , '%m/%d/%y %H:%M:%S'))
                except:
                    searchData = None
            elif searchField == 'name':
                field = TestRun.name
                searchData = unicode(searchData)
            else:
                field = TestRun.storeID
                try:
                    searchData = int(searchData)
                except:
                    searchData = None
            if searchData is not None:
                oper = None
                if searchOper == 'eq':
                    if searchField == 'date':
                        oper = attributes.AND(field < searchData,
                                              field > searchData.oneDay())
                    else:
                        oper = field == searchData
                elif searchOper == 'gt':
                    oper = field > searchData
                elif searchOper == 'lt':
                    if searchField == 'date':
                        searchData = searchData.oneDay()
                    oper = field < searchData
                elif searchOper == 'cn':
                    oper = field.like('%%%s%%' % searchData)
                if oper is not None:
                    total = db.count(TestRun,oper)
                    result = db.query(TestRun, oper,
                                      offset=(page-1)*size, limit=size,
                                      sort=TestRun.date.descending)

    if result is None:
        total = db.count(TestRun)
        result = db.query(TestRun, offset=(page-1)*size, limit=size,
                          sort=TestRun.date.descending)

    rows = []
    for test in list(result):
        rows.append(dict(id=test.storeID, cell=[time.strftime('%Y/%m/%d %H:%M:%S', test.date.asStructTime()), test.name]))
    if total == size:
        total_pages = total/size
    else:
        total_pages = (total/size) + 1
    data['records'] = len(rows)
    data['total'] = total_pages
    data['page'] = page
    data['rows'] = rows

    return static.Data(simplejson.dumps(data),
                       "application/json; charset=UTF-8",
                       expires=604800)

def testdb_logs_json(request, db):
    data = {}
    test_id = int(request.args.get('test_id',[0])[0])
    try:
        test = db.get_test(test_id)
    except NoSuchObjectError:
        return rend.FourOhFour()
    size = int(request.args.get('rows',[30])[0])
    page = int(request.args.get('page',[0])[0])

    rows = []
    if test:
        logs = test.get_logs(start=(page-1)*size, limit=size)
        total = test.get_logs_count()
    else:
        logs = []
        total = 0

    for line in logs:
        pdu_dict = line.pdu_dict()
        msg = line.pdu()
        if msg:
            msg_data = simplejson.dumps(msg.as_tree())
        else:
            msg_data = ''
        rows.append(dict(id=line.storeID,
            cell=[
                unicode(time.strftime('%Y/%m/%d %H:%M:%S', line.date.asStructTime())),
                line.level,
                line.msg,
                msg_data
            ]
    ))

    if total == size:
        total_pages = total/size
    else:
        total_pages = (total/size) + 1
    data['records'] = len(rows)
    data['total'] = total_pages
    data['page'] = page
    data['rows'] = rows

    return static.Data(simplejson.dumps(data),
                       "application/json; charset=UTF-8",
                       expires=604800)


def testdb_travel_kml(request, db):
    test_id = int(request.args.get('test_id',[0])[0])
    try:
        data = static.Data(
            str(IKMLTestReport(db).get_kml(test_id)),
            "application/vnd.google-earth.kml+xml; charset=UTF-8",
            expires=604800)
        return data
    except NoSuchObjectError:
        return rend.FourOhFour()

class TestdbGrid(page.Element):
    docFactory = loaders.xmlfile(templateDir=config.TEMPLATE_DIR,
                                 template='testdb_grid.html')

class TestdbPage(BasePage):
    def __init__(self, bench, authorizator=None):
        BasePage.__init__(self, authorizator=authorizator)
        self.bench = bench

    def renderHTTP(self, ctx):
        request = inevow.IRequest(ctx)

        script = request.prepath[-1]
        if not self.auth_http(request):
            request = inevow.IRequest(ctx)
            request.setHeader('WWW-Authenticate', 'Basic Realm="acpyd"')
            request.setResponseCode(http.UNAUTHORIZED)
            return "Authentication required."
        return rend.FourOhFour()

    def childFactory(self, ctx, name):
        request = inevow.IRequest(ctx)
        if not self.auth_http(request):
            return TestdbPage(self.bench, authorizator=self.authorizator)
        if name == 'list.json':
            return testdb_list_json(request, self.bench.testdb)
        elif name == 'edit.json':
            if 'id' in request.args:
                ids = map(int, request.args['id'][0].split(','))
                for id in ids:
                    test = self.bench.testdb.get_test(id)
                    if test is not None:
                        test.deleteFromStore()
                return ''
            else:
                return rend.FourOhFour()
        elif name == 'logs.json':
            return testdb_logs_json(request, self.bench.testdb)
        elif name == 'test.pdf':
            file_name = get_report_file_name (self.bench.reports_dir,
                                            int(request.args.get('id',[0])[0]))
            return static.File(file_name,"application/pdf")
        elif name == 'travels.kml':
            return testdb_travel_kml(request, self.bench.testdb)

        return rend.FourOhFour()

def get_report_file_name(dir, id):
    return (dir+"/"+str(id)+".pdf")


class PdfReportDialog(BaseElement, Observer, Observable):
    jsClass = u"acpyd.PdfRepGen"
    docFactory = loaders.xmlfile(templateDir=config.TEMPLATE_DIR,
                                 template='pdf_report_dialog.html',
                                 pattern='PdfReportDialogPattern')
    def __init__(self, bench):
        BaseElement.__init__(self)
        Observer.__init__(self, bench.getLiveObserver(self))
        Observable.__init__(self, bench.getLiveObserver(self))
        self.bench = bench
        self.db = bench.testdb
        self.prev_value=0
        self.cancel=False

    def pdf_report_progressbar(self, value):
        if self.cancel == True:
            self.cancel = False
            raise Exception ('PDF generation canceled by user')
        self.sendNotify('progress', value)

    def testdb_report_pdf_generate (self, test_id):
        self.cancel=False
        test_ids = [test_id]
        self.test_id = test_id
        self.file_name = get_report_file_name (self.bench.reports_dir,self.test_id)
        (head, tail)= os.path.split(self.file_name)
        if head.find('co_reports') > 0:
            mode = 'console'
        else:
            mode = 'gateway'
        """Verify if pdf file was generated already"""
        for file in os.listdir(head):
            if fnmatch.fnmatch(file,tail):
                self.callRemote ("pdf_report_finished", unicode(mode), unicode(str(self.test_id)))
                return
        def cb(data):
            self.callRemote ("pdf_report_finished", unicode(mode),unicode(str(self.test_id)))
        IPDFTestReport(self.db).get_pdf(test_ids, self.file_name,
                                        self.pdf_report_progressbar).addCallback(cb)
    athena.expose(testdb_report_pdf_generate)

    def testdb_report_pdf_cancel(self):
        value=''
        self.cancel = True
        """Send notification from Observable to Observers"""
        self.sendNotify('cancel', value)
    athena.expose(testdb_report_pdf_cancel)

    def notify(self, param, value, *args, **kwargs):
        """Observer method called by Observable"""
        if param == 'progress':
            if self.prev_value != value:
                self.prev_value = value
                self.callRemote("pdf_report_progressbar_update", unicode(str(value)))
        if param == 'cancel':
            self.callRemote("pdf_report_cancel_notify")
            if self.cancel == False:
                self.cancel = True

    def disconnected(self):
        self.unregister();
