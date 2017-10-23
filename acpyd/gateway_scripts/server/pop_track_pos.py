short_name = 'Pop Track Position'
description = """
Gets and removes the first Track Position message queued
by the server. The operation blocks until there's an available message.
"""

resp_msg = TrackPos

def script(conn, args):
    while True:
        msg = conn.pop_first(TrackPos)
        if msg:
            yield Result(msg)
        else:
            yield
