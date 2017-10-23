#!/usr/bin/python
import sys
import acpyd.test.test_gateway as gw
from twisted.internet import reactor, defer

def run_tests(host, port):
    gw.SERVER_HOST = host
    gw.SERVER_PORT = port
    print 'Running against %s:%s' % (host, port)
    test = gw.ACPGwTest()
    for member_name in dir(test):
        if member_name.startswith('test_'):
            member = getattr(test, member_name)
            print 'Running', member_name
            d = defer.Deferred()
            d_start_test = gw.base_req('start_test?name=%s' % member_name)
            def run(res):
                member().chainDeferred(d)
            def err(res):
                d.errback(res)
            d_start_test.addCallbacks(run, err)
            yield d

g = run_tests(sys.argv[1], int(sys.argv[2]))
def error(failure):
    print dir(failure), failure
    reactor.stop()

def stop_server(*args):
    d1 = gw.server_req('stop')
    d2 = gw.client_req('stop')
    defer.gatherResults([d1,d2])\
            .addCallback(run_next)\
            .addErrback(error)

def run_next(*args):
    try:
        d = g.next()
        d.addCallback(stop_server)
        d.addErrback(error)
    except StopIteration:
        reactor.stop()
run_next()
reactor.run()
