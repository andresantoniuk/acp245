# 13.4.5 Configuration TCU Service Activation/ Deactivation Message ACP 245 (Activation) - Test Case
# Client side.
# Receive CfgActivation and send CfgReply as reply.

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

    # wait for CfgActivation (13.4.5.1)
    msg = conn.pop_first(CfgActivation)
    while msg is None:
        if not conn.connected:
            fail("Didn't receive CfgActivation before disconnection")
        yield
        msg = conn.pop_first(CfgActivation)
    assert_equals(msg.ctrl_byte, 0x40)

    # Should match the vehicle descriptor sent on the first message
    assert_true(msg.vehicle_desc is not None)
    # ICCID must match the one sent on the request
    assert_equals(msg.vehicle_desc.iccid, vehicle_desc.iccid)

 
    # get standard reply (CfgReply)
    reply = stdmsg['13.4.5.2']

    reply.version = version

    # copy the vehicle descriptor of the request
    reply.vehicle_desc = msg.vehicle_desc

    # send the CfgReply)
    conn.send_msg(reply)
