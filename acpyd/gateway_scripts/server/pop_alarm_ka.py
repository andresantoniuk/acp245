short_name = 'Pop Alarm Keepalive'
description = """
Gets and removes the first KeepAlive message queued by the
server. The operation blocks until there's an available message.
"""

resp_msg = AlarmKA

def script(conn, args):
    while True:
        msg = conn.pop_first(AlarmKA)
        if msg:
            yield Result(msg)
        else:
            yield
