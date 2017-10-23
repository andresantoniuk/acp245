short_name = 'Send Alarm Notification'
description = """
Sends an Alarm Notification message to a connected SO. If no SO is connected, this
operation will block until one connects.
"""

arg_msg = AlarmNotif

def script(conn, args):
    msg = parse_args(args, AlarmNotif())

    # wait for reply
    conn.send_msg(msg)
