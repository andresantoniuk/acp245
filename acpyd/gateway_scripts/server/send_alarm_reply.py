short_name = 'Send Alarm Reply'
description = """
Sends an Alarm Reply message to a connected TCU. If no TCU is connected, this
operation will block until one connects.
"""

arg_msg = AlarmReply

def script(conn, args):
    msg = parse_args(args, AlarmReply())

    conn.send_msg(msg)
