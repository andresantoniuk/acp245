# CAUTION: script requires to follow indentation precisely.

def script(conn):
    # server received a connection
    msg = TrackReply(
        header=Header(
            app_id=ACP_APP_ID_VEHICLE_TRACKING,     # 0x0A
            test=0x00,                              # Test ? 
            type=ACP_MSG_TYPE_TRACK_REPLY,          # 0x03
            version=0x02                            # Version 1.2.2
        ),
        confirmation = 1,                           #TODO
        transmit_unit = 2,                          #TODO
        error = IEError(
            code=0
            ),                                      #TODO        
        
    )   

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

    assert_equals(msg.target_app_id, 0x0)

    # error is present
    assert_true(msg.error)
    # error code should be 0
    assert_equals(msg.error.code, 0)

    # vehicle descriptor is present
    assert_true(msg.vehicle_desc)

    # end of test
