# CAUTION: script requires to follow indentation precisely.

def script(conn):
    msg = FuncCmd(
        version=IEVersion(
            car_manufacturer=0x08,                  # Volkswagen
            tcu_manufacturer=0x82,                  # JCI
            major_hard_rel=0x01,
            major_soft_rel=0x03
        ),
        ctrl_func=IECtrlFunc(
            entity_id=ACP_ENT_ID_IMMOBILIZE,        # 0x0A
            transmit_unit=ACP_EL_TIME_UNIT_SECOND,  # 0
            transmit_interval=60,                   # 0x3C
        ),
        func_cmd=IEFuncCmd(
            cmd=ACP_FUNC_CMD_ENABLE,                # 0x02
        ),
        vehicle_desc=IEVehicleDesc(
            imei="0490154100837810",                 # replace with real IMEI
            iccid="08923440000000000003",            # replace with real ICCID
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
    # check that car manufacturer is Volkswagen (30)
    assert_equals(msg.version.car_manufacturer, 0x08)
    # check that TCU manufacturer is JCI (130)
    assert_equals(msg.version.tcu_manufacturer, 130)
    # check release numbers
    assert_equals(msg.version.major_hard_rel, 0x1)
    assert_equals(msg.version.major_soft_rel, 0x3)

    # error element is present
    assert_true(msg.error)
    # ACP_ERR_OK == 0 == OK
    assert_equals(msg.error.code, ACP_ERR_OK)
