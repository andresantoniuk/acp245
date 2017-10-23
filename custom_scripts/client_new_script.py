# 13.4.1 Updating Tracking Timer without version element - Test Case
# Client side.
# Receive CfgUpd245 and send CfgReply245 as reply.

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
    vehicle_desc = IEVehicleDesc(
        iccid   = '99999999999999999999',
        auth_key= '\x01\x02\x03\x04\x05\x06\x07\x08',
    )

    # send vehicle position message after reconnecting
    # as specified in 14.13.1
    start_msg = stdmsg['13.6.1.1']
    start_msg.version = version
    start_msg.vehicle_desc = vehicle_desc
    conn.send_msg(start_msg)

    # wait for CfgUpd245 (13.4.1.1)
    msg = conn.pop_first(CfgUpd245)
    while msg is None:
        if not conn.connected:
            fail("Didn't receive CfgUpd245 before disconnection")
        yield
        msg = conn.pop_first(CfgUpd245)
    assert_equals(msg.appl_flg, 0x03)
    assert_equals(msg.ctrl_flg1, 0x02)

    assert_equals(msg.tcu_data.items[0].type, 0x11)
    assert_equals(len(msg.tcu_data.items[0].data), 2)

    # Should match the vehicle descriptor sent on the first message
    assert_true(msg.vehicle_desc is not None)
    # ICCID must match the one sent on the request
    assert_equals(msg.vehicle_desc.iccid, vehicle_desc.iccid)

 
    # get standard reply (CfgReply245)
    reply = stdmsg['13.4.1.2']

    reply.version = version

    # copy the vehicle descriptor of the request
    reply.vehicle_desc = msg.vehicle_desc

    # send the CfgReply245)
    conn.send_msg(reply)
