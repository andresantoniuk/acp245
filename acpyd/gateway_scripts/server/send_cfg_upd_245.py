short_name = 'Send Configuration Update 245'
description = """
Sends a Configuration Update 245 message to a connected TCU. If no TCU is connected, this
operation will block until one connects. The operation waits for a reply and
returns it, discarding every other received message which is not a reply to this
one.
"""

arg_msg = CfgUpd245
resp_msg = CfgReply

def script(conn, args):
    msg = parse_args(args, CfgUpd245())

    # wait for reply
    conn.send_msg(msg)

    while not conn.has_reply(msg):
        conn.discard_all()
        yield

    yield Result(conn.pop_reply(msg))
