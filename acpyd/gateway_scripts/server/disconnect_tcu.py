short_name = 'Disconnect TCU'
description = """
Disconnect the connected TCU. If no TCU is connected, blocks until
a TCU connects to the server and then immediately disconnects it.
"""

response = (
    ('host',        String('', note='TCU IP or host name')),
    ('port',        Int16('', note='TCU Port number'))
)

def script(conn,args):
    host = conn.peer_name
    port = conn.peer_port
    conn.close()
    return {
        'host':host,
        'port':port
    }
