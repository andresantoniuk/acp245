# CAUTION: script requires to follow indentation precisely.

def script(conn):
    msg = AlarmKA(
        vehicle_desc=IEVehicleDesc(
            imei="0490154100837810",                 # replace with real IMEI
            iccid="08923440000000000003",            # replace with real ICCID
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
    assert_equals(msg.vehicle_desc.imei, "0490154100837810")
    assert_equals(msg.vehicle_desc.iccid, "08923440000000000003")
