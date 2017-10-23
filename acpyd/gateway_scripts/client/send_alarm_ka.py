short_name = 'Send alarm keepalive'
description = """
Sends an Alarm Keepalive message to a connected SO. If no SO is connected, this
operation will block until one connects.
"""

arg_msg = AlarmKA

def script(conn, args):
    msg = parse_args(args, AlarmKA())

    conn.send_msg(msg)
