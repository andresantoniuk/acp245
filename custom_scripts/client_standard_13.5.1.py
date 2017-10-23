# 13.5.1 Blocking request: no version element (shortest message) - Test Case
# Client side.
# Receive FuncCmd and send FuncStatus as reply.

def script(conn):
    # *** set these fields to the manufacturer specific values ***
    # By default, it uses the the version element values given on the
    # ACP 245 specification for the vehicle position message
    version = stdmsg['13.6.1.1'].version
    #version = IEVersion(
    #    tcu_manufacturer    = 0x00,
    #    car_manufacturer    = 0x00,
    #    major_hard_rel      = 0x00,
    #    major_soft_rel      = 0x00,
    #)
    vehicle_desc = stdmsg['13.6.1.1'].vehicle_desc
    #vehicle_desc = IEVehicleDesc(
    #    iccid   = '01234567890123456789',
    #    auth_key= '\x01\x02\x03\x04\x05\x06\x07\x08',
    #)

    # send vehicle position message after reconnecting
    # as specified in 14.13.1
    start_msg = stdmsg['13.6.1.1']
    start_msg.version = version
    start_msg.vehicle_desc = vehicle_desc
    conn.send_msg(start_msg)

    # wait for FuncCmd (13.5.1.1)
    msg = conn.pop_first(FuncCmd)
    while msg is None:
        if not conn.connected:
            fail("Didn't receive FuncCmd before disconnection")
        yield
        msg = conn.pop_first(FuncCmd)

    assert_equals(msg.ctrl_func.entity_id, 0x80)

    assert_equals(msg.func_cmd.cmd, 0x02)

    # Should match the vehicle descriptor sent on the first message
    assert_true(msg.vehicle_desc is not None)
    # ICCID must match the one sent on the request
    assert_equals(msg.vehicle_desc.iccid, vehicle_desc.iccid)

 
    # get standard reply (FuncStatus)
    reply = stdmsg['13.5.1.2']

    reply.version = version

    # copy the vehicle descriptor of the request
    reply.vehicle_desc = msg.vehicle_desc

    # send the FuncStatus)
    conn.send_msg(reply)
