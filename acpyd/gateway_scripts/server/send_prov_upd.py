short_name = 'Send Provision Update'
description = """
Sends a Provision Update message to a connected TCU. If no TCU is connected, this
operation will block until one connects. The operation waits for a reply and
returns it, discarding every other received message which is not a reply to this
one.
"""

arg_msg = ProvUpd
resp_msg = ProvReply

def script(conn, args):
    msg = parse_args(args, ProvUpd())
    # wait for reply
    conn.send_msg(msg)

    while not conn.has_reply(msg):
        conn.discard_all()
        yield

    yield Result(conn.pop_reply(msg))
