short_name = 'Wait for connection'
description = '''
Waits until the client connects to an SO.
'''

response = (
    ('host',          String('', note='TCU IP or host name')),
    ('port',          Int16('', note='TCU Port number'))
)

def script(conn, args):
    print 'CONNECTED to', conn.peer_name
    return {
        'host':conn.peer_name,
        'port':conn.peer_port
    }
