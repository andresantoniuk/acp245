# CAUTION: script requires to follow indentation precisely.

def script(conn):
    # server received a connection, send message
    msg = stdmsg['13.4.3.1']                       #Populates the message data as described in ACP245 Specification 13.4.3.1, 
                                                   #Config request for status of 0x0011 (tracking timer)
    msg.vehicle_desc.iccid='89314404000055312152'  #Override the data fields that are needed to be different
    msg.header.version=2                           

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
