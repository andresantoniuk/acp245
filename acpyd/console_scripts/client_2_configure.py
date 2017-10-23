# CAUTION: script requires to follow indentation precisely.

def script(conn):
    #wait for CfgUpd245 message
    yield
    # check that there are available messages
    assert_true(conn.messages)
    # get first message
    msg = conn.pop_msg(0)
    # check that is a configuration update 245 message
    assert_equals(msg.app_id, ACP_APP_ID_CONFIGURATION)
    assert_equals(msg.type, ACP_MSG_TYPE_CFG_UPD_245)

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

    assert_equals(msg.target_app_id, 0x1)
    # start time is not present
    assert_false(msg.start_time)
    # end time is present
    assert_true(msg.end_time)

    # check end time
    assert_equals(msg.end_time.year, 2009)
    assert_equals(msg.end_time.month, 03)
    assert_equals(msg.end_time.day, 20)
    assert_equals(msg.end_time.hour, 01)
    assert_equals(msg.end_time.minute, 03)
    assert_equals(msg.end_time.second, 04)

    # grace time is not present
    assert_false(msg.grace_time)

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

    # TCU descriptor is present
    assert_true(msg.tcu_desc)
    assert_equals(msg.tcu_desc.device_id, 0x20)
    assert_equals(msg.tcu_desc.version, "0.0.1")

    # TCU data is present
    assert_true(msg.tcu_data)
    # 3 TCU data items received
    assert_equals(len(msg.tcu_data.items), 3)

    # See ACP 245 specification, Appendix I section 12.1 for valid
    # parameters.
    # check that the first item is transmission interval (0x0011)
    assert_equals(msg.tcu_data.items[0].type, 0x11)
    # value should be 300 seconds (0x12C)
    assert_equals(msg.tcu_data.items[0].data, (0x01, 0x2C))

    # check that the second one is the server IP
    assert_equals(msg.tcu_data.items[1].type, 0x64)
    # value should be 209.20.85.61 (jci.edantech.com) (0xD114553D)
    assert_equals(msg.tcu_data.items[1].data, (0xD1,0x14,0x55,0x3D))

    # check that the second one is the server port
    assert_equals(msg.tcu_data.items[2].type, 0x65)
    # value should be 12001 (0x2EE1)
    assert_equals(msg.tcu_data.items[2].data, (0x2E, 0xE1))

    # Send a Configuration Update Reply
    reply = CfgReply(
        # reply with the same fields
        version=msg.version,
        target_app_id=msg.target_app_id,
        # No error
        error=IEError(code=0),
        vehicle_desc=msg.vehicle_desc
    )
    # send msg
    conn.send_msg(reply)

    # end of test
