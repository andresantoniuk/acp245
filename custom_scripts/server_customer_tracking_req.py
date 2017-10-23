# CAUTION: script requires to follow indentation precisely.

def script(conn):
    msg = FuncCmd(
        header=Header(
            app_id=0x06,                            # Remote Vehicle Function Service
            test=0x00,                              # Test ? 
            type=0x02,                              # Type
            version=0x02                            # Version
        ),
        version=IEVersion(
            car_manufacturer=0x22,                  # JLR ( 34 = 0x22 )
            tcu_manufacturer=0x90,                  # OTHERS
            major_hard_rel=0x01,
            major_soft_rel=0x01
        ),
        ctrl_func=IECtrlFunc(
            entity_id=0x01,        # 01=Vehicle Tracking, 07=Tracking with Commit, 08=Commit to Tracking
            transmit_unit=ACP_EL_TIME_UNIT_SECOND,  # 0
            transmit_interval=60,                   # 0x3C
        ),
        func_cmd=IEFuncCmd(
            cmd=ACP_FUNC_CMD_ENABLE,                # 0x02=Start, 0x03=Stop
        ),
        vehicle_desc=IEVehicleDesc(
            imei="354476050013071",                 # replace with real IMEI
            iccid="89314404000055312152",            # replace with real ICCID
        ),
    )
    conn.send_msg(msg)
    # wait for remote vehicle function reply
    yield

    # check that there are available messages
    assert_true(conn.messages)
    # get first message
    msg = conn.pop_msg(0)
    # check that is a remote vehicle function reply
    assert_equals(msg.app_id, ACP_APP_ID_REMOTE_VEHICLE_FUNCTION)
    assert_equals(msg.type, ACP_MSG_TYPE_FUNC_STATUS)

    # version element is present
    assert_true(msg.version)
    # check that car manufacturer is Jaguar (06)
    assert_equals(msg.version.car_manufacturer, 0x06)
    # check that TCU manufacturer is OTHERS(144)
    assert_equals(msg.version.tcu_manufacturer, 0x90)
    # check release numbers
    assert_equals(msg.version.major_hard_rel, 0x1)
    assert_equals(msg.version.major_soft_rel, 0x1)

    # error element is present
    assert_true(msg.error)
    # ACP_ERR_OK == 0 == OK
    print msg.error.code
    # assert_equals(msg.error.code, ACP_ERR_OK)
