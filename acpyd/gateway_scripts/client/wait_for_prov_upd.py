short_name = 'Wait for Provision Request'
description = '''
Waits for a Provision Request message and sends a Provision Reply to the
connected SO.
The parameters of this command indicate the field values of the reply.
'''

arg_msg = ProvReply
resp_msg = ProvUpd

def script(conn, args):
    reply = parse_args(args, ProvReply())

    msg = conn.pop_first(ProvUpd)
    while not msg:
        yield
        msg = conn.pop_first(ProvUpd)

    conn.send_msg(reply)

    yield Result(msg)
