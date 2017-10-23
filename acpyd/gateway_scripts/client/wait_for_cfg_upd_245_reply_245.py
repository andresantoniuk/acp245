short_name = 'Wait for Configure Update 245, reply with Reply 245'
description = '''
Waits for a Configure Update 245 message and sends a Configure Reply 245 to the
connected SO.
The parameters of this command indicate the field values of the reply.
'''

arg_msg = CfgReply245
resp_msg = CfgUpd245

def script(conn, args):
    reply = parse_args(args, CfgReply245())

    msg = conn.pop_first(CfgUpd245)
    while not msg:
        yield
        msg = conn.pop_first(CfgUpd245)

    conn.send_msg(reply)
    yield Result(msg)
