# CAUTION: script requires to follow indentation precisely.

def script(conn):
    #wait for TrackCmd message
    yield
    # check that there are available messages
    assert_true(conn.messages)
    # get first message
    msg = conn.pop_msg(0)
    # check that is a vehicle tracking command message
    assert_equals(msg.app_id, ACP_APP_ID_VEHICLE_TRACKING)
    assert_equals(msg.type, ACP_MSG_TYPE_TRACK_CMD)

    # check that the test flag was not set on the header
    assert_false(msg.header.test)

    # the version element is present
    assert_true(msg.version)
    # check that car manufacturer is Effa Motors (30)
    assert_equals(msg.version.car_manufacturer, 0x20)
    # check that TCU manufacturer is JCI (130)
    assert_equals(msg.version.tcu_manufacturer, 130)
    # check release numbers
    assert_equals(msg.version.major_hard_rel, 0x1)
    assert_equals(msg.version.major_soft_rel, 0x2)

    # vehicle descriptor is present
    assert_true(msg.vehicle_desc)
    # check VIN
    assert_equals(msg.vehicle_desc.vin, "123456789012345")
    # IMEI not present
    assert_false(msg.vehicle_desc.imei)
    # ICCID not present
    assert_false(msg.vehicle_desc.iccid)
    # Authentication Key not present
    assert_false(msg.vehicle_desc.auth_key)

    # Control function is present
    assert_true(msg.ctrl_func)
    assert_equals(msg.ctrl_func.entity_id, ACP_ENT_ID_VEHICLE_TRACK)
    assert_equals(msg.ctrl_func.transmit_interval, 10)
    assert_equals(msg.ctrl_func.transmit_unit, ACP_EL_TIME_UNIT_SECOND)

    # Function command is present
    assert_true(msg.func_cmd)
    assert_equals(msg.func_cmd.cmd, ACP_FUNC_CMD_ENABLE)

    # Send a Configuration Update Reply
    reply = TrackPos(
        # reply with the same fields
        version=msg.version,
        timestamp=IETimestamp(
            year=2009,
            month=03,
            day=20,
            hour=12,
            minute=30,
            second=20
        ),
        location=IELocation(
            curr_gps=IEGPSRawData(
                lon=10,
                lat=20,
                velocity=30,
                satellites=[1,2,3]
            )
        ),
        vehicle_desc=msg.vehicle_desc
    )
    # send msg
    conn.send_msg(reply)

    # wait for track reply
    yield
    # check that there are available messages
    assert_true(conn.messages)
    # get first message
    msg = conn.pop_msg(0)

    # check that is a vehicle tracking reply message
    assert_equals(msg.app_id, ACP_APP_ID_VEHICLE_TRACKING)
    assert_equals(msg.type, ACP_MSG_TYPE_TRACK_REPLY)

    # error is present
    assert_true(msg.error)

    # error code is OK
    assert_equals(msg.error.code, 0)

    # the version element is present
    assert_true(msg.version)
    # check that car manufacturer is Effa Motors (30)
    assert_equals(msg.version.car_manufacturer, 0x20)
    # check that TCU manufacturer is JCI (130)
    assert_equals(msg.version.tcu_manufacturer, 130)
    # check release numbers
    assert_equals(msg.version.major_hard_rel, 0x1)
    assert_equals(msg.version.major_soft_rel, 0x2)

    # end of test
