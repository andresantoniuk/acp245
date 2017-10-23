# CAUTION: script requires to follow indentation precisely.

def script(conn):
    # server received a connection, send message
    msg = TrackCmd(
        version=IEVersion(
            car_manufacturer=0x20,# Car manufacturer is Effa Motors (30)
            tcu_manufacturer=130, # TCU manufacturer is JCI (130)
            # release numbers
            major_hard_rel=0x1,
            major_soft_rel=0x2
        ),
        ctrl_func=IECtrlFunc(
            entity_id=ACP_ENT_ID_VEHICLE_TRACK,
            transmit_interval=10,
            transmit_unit=ACP_EL_TIME_UNIT_SECOND,
        ),
        func_cmd=IEFuncCmd(
            cmd=ACP_FUNC_CMD_ENABLE
        ),
        vehicle_desc=IEVehicleDesc(
            vin="123456789012345",
            # do not include, IMEI, ICCID or Authentication Key
        )
    ) # end of TrackCmd

    # send msg
    conn.send_msg(msg)

    # wait for client vehicle position
    yield

    # check that there are available messages
    assert_true(conn.messages)
    # get first message
    msg = conn.pop_msg(0)
    # check that is a vehicle position message
    assert_equals(msg.app_id, ACP_APP_ID_VEHICLE_TRACKING)
    assert_equals(msg.type, ACP_MSG_TYPE_TRACK_POS)

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

    # location element is present
    assert_true(msg.location)
    # current gps data is present
    assert_true(msg.location.curr_gps)
    # current gps data is present
    assert_equals(msg.location.curr_gps.lon, 10)
    # current gps data is present
    assert_equals(msg.location.curr_gps.lat, 20)
    # current gps data is present
    assert_equals(msg.location.curr_gps.velocity, 30)
    # current gps data is present
    assert_equals(msg.location.curr_gps.satellites, [1,2,3])


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

    # send track reply
    reply = TrackReply(
        version = msg.version,
        error = IEError(code=0)
    )
    conn.send_msg(reply)

    # end of test
