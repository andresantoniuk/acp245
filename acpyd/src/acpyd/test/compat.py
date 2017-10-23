import sys
if sys.platform == 'win32':
    # Set SO_REUSEADDR so tests cases do not fail
    # because socket is in use.
    # In theory, this shouldn't be needed according to
    # http://twistedmatrix.com/trac/ticket/1151
    # but it solves the problem. Maybe the problem only
    # happens when running the tests cases with wine over
    # linux?
    from twisted.internet.tcp import Port
    import socket
    createInternetSocket_orig = Port.createInternetSocket
    def createInternetSocket_win(*args, **kwargs):
        s = createInternetSocket_orig(*args, **kwargs)
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        return s
    Port.createInternetSocket = createInternetSocket_win
