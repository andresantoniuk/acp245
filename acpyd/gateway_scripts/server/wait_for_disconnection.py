short_name = 'Wait for TCU disconnection'
description = """
Blocks until a TCU disconnects from the server. If a TCU is not connected, the
operation blocks until one connects and then disconnects.
"""

response = (
    ('host',          String('', note='TCU IP or host name')),
    ('port',          Int16('', note='TCU Port number'))
)

def script(conn,args):
    peer = {
        'host':conn.peer_name,
        'port':conn.peer_port
    }
    while conn.connected:
        yield
    yield Result(peer)
