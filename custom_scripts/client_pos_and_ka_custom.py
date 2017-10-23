# CAUTION: script requires to follow indentation precisely.

def script(conn):
    version = stdmsg['13.6.1.1'].version
    vehicle_desc = stdmsg['13.6.1.1'].vehicle_desc
    vehicle_desc.iccid = '12345678901234567890'

    msg = stdmsg['13.6.1.2']
    msg.version = version
    msg.vehicle_desc = vehicle_desc

    # send the TrackPos
    conn.send_msg(msg)

    yield
    msg = AlarmKA(
        vehicle_desc=IEVehicleDesc(
            iccid="12345678901234567890",            # replace with real ICCID
        )
    )
    conn.send_msg(msg)
    # wait for KA reply
    yield

    # check that there are available messages
    assert_true(conn.messages)
    # get first message
    msg = conn.pop_msg(0)

    # check that is a remote vehicle function reply
    assert_equals(msg.app_id, ACP_APP_ID_ALARM)
    assert_equals(msg.type, ACP_MSG_TYPE_ALARM_KA_REPLY)
    assert_equals(msg.vehicle_desc.iccid, "12345678901234567890")
