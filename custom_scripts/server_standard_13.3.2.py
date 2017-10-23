# 13.3.2 Consult request: no Version Element - Test Case
# Server side.
# Send ProvUpd and receive ProvReply as reply.

def script(conn):
    # wait for reconnection message from TCU, as specified on 14.13.1
    while not conn.messages:
        if not conn.connected:
            fail("Didn't receive start message before disconnection")
        yield

    start_msg = conn.pop_first()
    # check that first message includes a verion element and ICCID
    assert_true(start_msg.is_valid_tcu_first_msg())

    # store the version and vehicle descriptor element for later use
    version = start_msg.version
    vehicle_desc = start_msg.vehicle_desc

    # get standard request (ProvUpd)
    msg = stdmsg['13.3.2.1']

    # use the same vehicle descriptor as the one received from the TCU
    msg.vehicle_desc = vehicle_desc

    # send the ProvUpd
    conn.send_msg(msg)

    # wait for ProvReply
    reply = conn.pop_first(ProvReply)
    while reply is None:
        if not conn.connected:
            fail("Didn't receive ProvReply before disconnection")
        yield
        reply = conn.pop_first(ProvReply)

    assert_equals(reply.target_app_id,  0x0B)
    assert_equals(reply.ctrl_flg1, 0x02)

    assert_equals(reply.tcu_resp, 0x02)

    # ICCID must match the one received on the first message
    assert_true(reply.vehicle_desc is not None)
    assert_equals(reply.vehicle_desc.iccid, vehicle_desc.iccid)

 
