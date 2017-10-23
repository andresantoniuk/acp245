# CAUTION: script requires to follow indentation precisely.

def script(conn):
    msg = ProvUpd(
       header=Header(
            app_id=0x01,                            # Provisioning 
            type=0x01,                              # Type
            version=0x2                             # Version 0x02=ACP245 1.2.2, part of the Header class
        ),
       #app_id=ACP_APP_ID_PROVISIONING,         # 0x01, Provisioning
       #type=ACP_MSG_TYPE_PROV_UPD,             # 0x01, Type
       #version=0x02                            # Version 0x02=ACP245 1.2.2, part of the Header class

        version=IEVersion(
            car_manufacturer=0x22,                  # JLR ( 34 = 0x22 )
            tcu_manufacturer=0x8e,                  # OTHERS
            major_hard_rel=0x01,
            major_soft_rel=0x01
        ),
        target_app_id=0x0B,                        # 10 - Tracking, 11 - Theft Alarm
        appl_flg=0x1,                              # Activate

        start_time=IETimestamp(
            year=2013,
            month=5,
            day=20
        ),
    )
    conn.send_msg(msg)
    # wait for provisioning message reply
    yield

    # check that there are available messages
    assert_true(conn.messages)
    # get first message
    msg = conn.pop_msg(0)
    # check that is a provisioning reply
    assert_equals(msg.app_id, ACP_APP_ID_PROVISIONING)

    # version element is present
    assert_true(msg.version)
    # check that car manufacturer is Land Rover (44)
    assert_equals(msg.version.car_manufacturer, 0x2C)
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
