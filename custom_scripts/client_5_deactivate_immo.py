# CAUTION: script requires to follow indentation precisely.

def script(conn):
    # connected, wait for remote vehicle function command
    yield

    # check that there are available messages
    assert_true(conn.messages)
    msg = conn.pop_msg(0)

    # check that is a remote vehicle function command
    assert_equals(msg.app_id, ACP_APP_ID_REMOTE_VEHICLE_FUNCTION)
    assert_equals(msg.type, ACP_MSG_TYPE_FUNC_CMD)

    # version element is present
    assert_true(msg.version)
    assert_equals(msg.version.car_manufacturer, 0x08)
    assert_equals(msg.version.tcu_manufacturer, 130)
    assert_equals(msg.version.major_hard_rel, 0x1)
    assert_equals(msg.version.major_soft_rel, 0x3)

    # control function command element is present
    assert_true(msg.ctrl_func)
    assert_equals(msg.ctrl_func.entity_id, ACP_ENT_ID_IMMOBILIZE)
    assert_equals(msg.ctrl_func.transmit_unit, ACP_EL_TIME_UNIT_SECOND)
    assert_equals(msg.ctrl_func.transmit_interval, 60)

    # function command element is present
    assert_true(msg.func_cmd)
    assert_equals(msg.func_cmd.cmd, ACP_FUNC_CMD_DISABLE)

    # vehicle descriptor element is present
    assert_true(msg.vehicle_desc)
    assert_equals(msg.vehicle_desc.imei, "0490154100837810")
    assert_equals(msg.vehicle_desc.iccid, "08923440000000000003")

    # send reply
    reply = FuncStatus(
        version = msg.version,
        ctrl_func = msg.ctrl_func,
        func_status = IEFuncCmd(            # func status and func 
                                            # cmd use the same IE
                                            # on ACP 245
            cmd=ACP_FUNC_STATE_DISABLED,    # 0x03, Immo disabled
        ),
        error = IEError(code=ACP_ERR_OK),
        vehicle_desc = msg.vehicle_desc
    )
    conn.send_msg(reply)
