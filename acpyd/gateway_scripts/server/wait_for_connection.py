short_name = 'Wait for TCU connection'
description = """
Blocks until a connection is established from a TCU. If a connection is already
established, the operation returns immediately.
"""

response = (
    ('host',          String('', note='TCU IP or host name')),
    ('port',          Int16('', note='TCU Port number'))
)

def script(conn,args):
    return {
        'host':conn.peer_name,
        'port':conn.peer_port
    }
