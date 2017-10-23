# Sends a configuration reply as the one sent after a TIV activates a TCU
# via SMS

def script(conn):
    bench.debug('sending configuration reply')
    # use the example configuration reply provided on the ACP245 document
    msg = stdmsg['13.4.5.2']
    conn.send_msg(msg)
    while conn.connected:
        yield
