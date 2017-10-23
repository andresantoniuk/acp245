# 13.4.4 Update Vehicle Location Data: no version element - Test Case
# Server side.
# Send CfgUpd245 and receive CfgReply245 as reply.

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

    # get standard request (CfgUpd245)
    msg = stdmsg['13.4.4.3']

    # use the same vehicle descriptor as the one received from the TCU
    msg.vehicle_desc = vehicle_desc

    # send the CfgUpd245
    conn.send_msg(msg)

    # wait for CfgReply245
    reply = conn.pop_first(CfgReply245)
    while reply is None:
        if not conn.connected:
            fail("Didn't receive CfgReply245 before disconnection")
        yield
        reply = conn.pop_first(CfgReply245)

    assert_equals(reply.appl_flg, 0x03)
    assert_equals(reply.ctrl_flg1, 0x02)

    # Version Element should match the version element of the first message.
    if reply.version is not None:
        assert_equals(reply.version.tcu_manufacturer, version.tcu_manufacturer)
        assert_equals(reply.version.car_manufacturer, version.car_manufacturer)
        assert_equals(reply.version.major_hard_rel, version.major_hard_rel)
        assert_equals(reply.version.major_soft_rel, version.major_soft_rel)

    if reply.status not in (0, 2, 3):
        fail('tcu_status must be 0, 2 or 3, got: %s' % reply.status)

    assert_equals(reply.error.items[0].type, 0x47)
    assert_equals(reply.error.items[0].error.code, 0x00)

    # ICCID must match the one received on the first message
    assert_true(reply.vehicle_desc is not None)
    assert_equals(reply.vehicle_desc.iccid, vehicle_desc.iccid)

 
