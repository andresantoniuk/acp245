short_name = 'Wait for Function Command'
description = '''
Waits for a Function Command message and sends a Function Status to the
connected SO.
The parameters of this command indicate the field values of the reply.
'''

arg_msg = FuncStatus
resp_msg = FuncCmd

def script(conn, args):
    reply = parse_args(args, FuncStatus())

    msg = conn.pop_first(FuncCmd)
    while not msg:
        yield
        msg = conn.pop_first(FuncCmd)

    conn.send_msg(reply)
    yield Result(msg)
