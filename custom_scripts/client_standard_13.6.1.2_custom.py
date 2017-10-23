# 13.6.1.2 Vehicle Position Message without Vehicle Location Data (Data type 0x0047 = False) - Test Case
# Client side.
# Send TrackPos

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
    vehicle_desc.iccid = '12345678901234567890'
    #vehicle_desc = IEVehicleDesc(
    #    iccid   = '01234567890123456789',
    #    auth_key= '\x01\x02\x03\x04\x05\x06\x07\x08',
    #)

    # get standard request (TrackPos)
    msg = stdmsg['13.6.1.2']

    msg.version = version

    msg.vehicle_desc = vehicle_desc

    # send the TrackPos
    conn.send_msg(msg)

