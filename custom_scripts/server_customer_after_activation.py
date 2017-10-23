# Waits for a configuration reply that must be sent after a TIV activates a TCU
# via SMS

def script(conn):
    bench.debug('wait for configuration message')
    yield

    msg = conn.pop_first(CfgReply)
    assert_true(msg is not None, 'no configuration reply received')

    assert_true(msg.is_valid_tcu_first_msg())
    assert_equals(msg.appl_flg, ACP_MSG_PROV_ACTIVATE)
    assert_equals(msg.error.code, 0)
