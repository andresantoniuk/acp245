short_name = 'Wait for Configure Update 245'
description = '''
Waits for a Configure Update 245 message and sends a Configure Reply to the
connected SO.
The parameters of this command indicate the field values of the reply.
'''

arg_msg = CfgReply
resp_msg = CfgUpd245

def script(conn, args):
    reply = parse_args(args, CfgReply())

    msg = conn.pop_first(CfgUpd245)
    while not msg:
        yield
        msg = conn.pop_first(CfgUpd245)

    conn.send_msg(reply)
    yield Result(msg)
