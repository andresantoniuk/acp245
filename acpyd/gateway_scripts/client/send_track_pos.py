short_name = 'Send Track Position'
description = """
Sends an Track Position message to a connected SO. If no SO is connected, this
operation will block until one connects.
"""

arg_msg = TrackPos

def script(conn, args):
    msg = parse_args(args, TrackPos())

    conn.send_msg(msg)
