short_name = 'Pop Track Reply'
description = """
Gets and removes the first Track Reply message queued
by the server. The operation blocks until there's an available message.
"""

resp_msg = TrackReply

def script(conn, args):
    while True:
        msg = conn.pop_first(TrackReply)
        if msg:
            yield Result(msg)
        else:
            yield
