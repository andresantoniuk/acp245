# CAUTION: script requires to follow indentation precisely.

def script(conn):
    while True:
        # wait for alarm keep-alive
        yield
        if not conn.connected:
            break

        # check that there are available messages
        assert_true(conn.messages)
        # get first message
        msg = conn.pop_msg(0)

        # check that is a remote vehicle function reply
        assert_equals(msg.app_id, ACP_APP_ID_ALARM)
        assert_equals(msg.type, ACP_MSG_TYPE_ALARM_KA)
        assert_equals(msg.vehicle_desc.imei, "0490154100837810")
        assert_equals(msg.vehicle_desc.iccid, "08923440000000000003")

        reply = AlarmKAReply(
            vehicle_desc=msg.vehicle_desc
        )

        # send keep-alive reply
        conn.send_msg(reply)
