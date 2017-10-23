# 13.7.1.4 Theft Alarm in event mode Vehicle Position Message with Vehicle Location Data (Data type 0x0047 = True) - Test Case
# Client side.
# Send AlarmPos

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

    # get standard request (AlarmPos)
    msg = stdmsg['13.7.1.4']

    msg.version = version

    msg.vehicle_desc = vehicle_desc

    # send the AlarmPos
    conn.send_msg(msg)

