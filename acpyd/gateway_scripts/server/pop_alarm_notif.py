short_name = 'Pop Alarm Notification'
description = """
Gets and removes the first Alarm Notification message queued
by the server. The operation blocks until there's an available message.
"""

resp_msg = AlarmNotif

def script(conn, args):
    while True:
        msg = conn.pop_first(AlarmNotif)
        if msg:
            yield Result(msg)
        else:
            yield
