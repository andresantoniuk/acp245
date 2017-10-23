# 13.5.2 Unblocking request: no version element (shortest message) - Test Case
# Server side.
# Send FuncCmd and receive FuncStatus as reply.

def script(conn):
    # wait for reconnection message from TCU, as specified on 14.13.1
    while not conn.messages:
        if not conn.connected:
            fail("Didn't receive start message before disconnection")
        yield

    start_msg = conn.pop_first()
    # check that first message includes a verion element and ICCID
    # assert_true(start_msg.is_valid_tcu_first_msg())

    # store the version and vehicle descriptor element for later use
    version = start_msg.version
    vehicle_desc = start_msg.vehicle_desc

    # get standard request (FuncCmd)
    msg = stdmsg['13.5.2.1']

    # use the same vehicle descriptor as the one received from the TCU
    msg.vehicle_desc = vehicle_desc

    # send the FuncCmd
    conn.send_msg(msg)

    # wait for FuncStatus
    reply = conn.pop_first(FuncStatus)
    while reply is None:
        if not conn.connected:
            fail("Didn't receive FuncStatus before disconnection")
        yield
        reply = conn.pop_first(FuncStatus)


    # Version Element should match the version element of the first message.
    if reply.version is not None:
        assert_equals(reply.version.tcu_manufacturer, version.tcu_manufacturer)
        assert_equals(reply.version.car_manufacturer, version.car_manufacturer)
        assert_equals(reply.version.major_hard_rel, version.major_hard_rel)
        assert_equals(reply.version.major_soft_rel, version.major_soft_rel)

    assert_equals(reply.ctrl_func.entity_id, 0x80)

    assert_equals(reply.func_status.cmd, 0x03)

    # ICCID must match the one received on the first message
    assert_true(reply.vehicle_desc is not None)
    assert_equals(reply.vehicle_desc.iccid, vehicle_desc.iccid)

 
