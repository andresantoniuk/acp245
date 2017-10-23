# CAUTION: script requires to follow indentation precisely.

def script(conn):
    msg = FuncCmd(
        header=Header(
            app_id=0x0B,                            # Vehicle Alarm
            test=0x00,                              # Test ? 
            type=0x02,                              # Type
            version=0x02                            # Version
        ),
        version=IEVersion(
            car_manufacturer=0x22,                  # JLR ( 34 = 0x22 )
            tcu_manufacturer=0x8e,                  # OTHERS
            major_hard_rel=0x01,
            major_soft_rel=0x01
        ),
        ctrl_func=IECtrlFunc(
            entity_id=ACP_ENT_ID_VEHICLE_BLOCK,     # 0x??
            transmit_unit=ACP_EL_TIME_UNIT_SECOND,  # 0
            transmit_interval=60,                   # 0x3C
        ),
        func_cmd=IEFuncCmd(
            cmd=ACP_FUNC_CMD_ENABLE,                # 0x02
        ),
        vehicle_desc=IEVehicleDesc(
            imei="0490154100837810",                 # replace with real IMEI
            iccid="89460800120005330798",            # replace with real ICCID
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
    print "Function Status = ", msg.type
    #assert_equals(msg.type, ACP_MSG_TYPE_FUNC_STATUS)

    # version element is present
    assert_true(msg.version)
    # check that car manufacturer is Volkswagen (30)
    assert_equals(msg.version.car_manufacturer, 0x06)
    # check that TCU manufacturer is OTHERS(142)
    assert_equals(msg.version.tcu_manufacturer, 0x8e)
    # check release numbers
    assert_equals(msg.version.major_hard_rel, 0x1)
    assert_equals(msg.version.major_soft_rel, 0x1)

    # error element is present
    #assert_true(msg.error)
    # ACP_ERR_OK == 0 == OK
    if hasattr(msg, 'error'):
        print "Error Code = ", msg.error.code
    # assert_equals(msg.error.code, ACP_ERR_OK)
