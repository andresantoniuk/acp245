short_name = 'Wait for Track Command'
description = '''
Waits for a Track Command message and sends a Track Position to the
connected SO.
The parameters of this command indicate the field values of the reply.
'''

arg_msg = TrackPos
resp_msg = TrackCmd

def script(conn, args):
    reply = parse_args(args, TrackPos())

    msg = conn.pop_first(TrackCmd)
    while not msg:
        yield
        msg = conn.pop_first(TrackCmd)

    conn.send_msg(reply)
    yield Result(msg)
