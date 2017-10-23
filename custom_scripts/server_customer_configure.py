# CAUTION: script requires to follow indentation precisely.

def script(conn):
    # server received a connection, send message
    msg = CfgUpd245(
        header=Header(
            app_id=0x02,                            # Configuration
            test=0x00,                              # Test ? 
            type=0x08,                              # Type
            version=0x2                             # Version
        ),
        target_app_id=0x01,

        vehicle_desc=IEVehicleDesc(
            vin="123456789012345",
            iccid="89314404000055312152"
            # do not include, IMEI, ICCID or Authentication Key
        ),
        tcu_data=IETCUData(
            items=[
                IETCUDataItem(
                    type=0x11,        # transmission interval (0x0011)
                    data=(0x00, 0x2C), # 300 seconds (0x2C)
                ),               
            ]
        )
    ) # end of CfgUpd245    

    # send msg
    conn.send_msg(msg)

    # wait for Client Reply
    yield

    # check that there are available messages
    assert_true(conn.messages)
    # get first message
    msg = conn.pop_msg(0)
    # check that is a configuration update 245 message
    assert_equals(msg.app_id, ACP_APP_ID_CONFIGURATION)
    assert_equals(msg.type, ACP_MSG_TYPE_CFG_REPLY_245)

    # check that the test flag was not set on the header
    assert_false(msg.header.test)

    # the version element is present
    assert_true(msg.version)
    # check that car manufacturer is Jaguar (6)
    assert_equals(msg.version.car_manufacturer, 0x06)
    # check that TCU manufacturer is JCI (142)
    assert_equals(msg.version.tcu_manufacturer, 142)
    # check release numbers
    assert_equals(msg.version.major_hard_rel, 0x1)
    assert_equals(msg.version.major_soft_rel, 0x1)

    assert_equals(msg.target_app_id, 0x0)

    # error is present
    assert_true(msg.error)
    # error code should be 0
    assert_equals(msg.error.code, 0)

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

    # end of test
