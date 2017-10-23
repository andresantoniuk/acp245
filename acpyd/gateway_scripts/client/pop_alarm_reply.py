short_name = 'Pop Alarm Reply'
description = """
Gets and removes the first Alarm Reply message queued
by the server. The operation blocks until there's an available message.
"""

resp_msg = AlarmReply

def script(conn, args):
    while True:
        msg = conn.pop_first(AlarmReply)
        if msg:
            yield Result(msg)
        else:
            yield
