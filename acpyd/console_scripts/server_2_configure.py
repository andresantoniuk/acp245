# CAUTION: script requires to follow indentation precisely.

def script(conn):
    # server received a connection, send message
    msg = CfgUpd245(
        version=IEVersion(
            car_manufacturer=0x20,# Car manufacturer is Effa Motors (30)
            tcu_manufacturer=130, # TCU manufacturer is JCI (130)
            # release numbers
            major_hard_rel=0x1,
            major_soft_rel=0x2
        ),
        target_app_id=0x1,
        end_time=IETimestamp(
            year=2009,
            month=03,
            day=20,
            hour=01,
            minute=03,
            second=04
        ),

        # do not include start time or grace time

        vehicle_desc=IEVehicleDesc(
            vin="123456789012345",
            # do not include, IMEI, ICCID or Authentication Key
        ),
        tcu_desc=IETCUDesc(
            device_id=0x20,
            version="0.0.1"
        ),
        tcu_data=IETCUData(
            items=[
                IETCUDataItem(
                    type=0x11,      # transmission interval (0x0011)
                    data='\x01\x2C' # 300 seconds (0x12C)
                    # you could also write: data=(0x01,0x2C)
                ),
                IETCUDataItem(
                    type=0x64,                  # server IP
                    data='\xD1\x14\x55\x3D'  # 209.20.85.61 (0xD114553D)
                ),
                IETCUDataItem(
                    type=0x65,      # server Port
                    data=(0x2E,0xE1) # 12001 (0x2EE1)
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
    assert_equals(msg.type, ACP_MSG_TYPE_CFG_REPLY)

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
