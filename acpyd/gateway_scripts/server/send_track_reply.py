short_name = 'Send Track Reply'
description = """
Sends a Track Reply message to a connected TCU. If no TCU is connected, this
operation will block until one connects.
"""

arg_msg = TrackReply

def script(conn, args):
    msg = parse_args(args, TrackReply())

    # wait for reply
    conn.send_msg(msg)
    return 'OK'
