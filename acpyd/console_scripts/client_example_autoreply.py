# CAUTION: script requires to follow indentation precisely.

# This example sends an Alarm KA and waits for a reply using the
# autoreply feature
def script(conn):
    msg = AlarmKA(
        vehicle_desc=IEVehicleDesc(
            imei="0490154100837810",                 # replace with real IMEI
            iccid="08923440000000000003",            # replace with real ICCID
        )
    )
    conn.send_msg(msg)

    # a condition
    # returns true if the msg is an instance of an AlarmKAReply
    def is_alarm_ka_reply(msg):
        return isinstance(msg, AlarmKAReply)

    # a function to check if the reply is valid
    def check_alarm_ka_reply(msg):
        # check that is a remote vehicle function reply
        assert_equals(msg.app_id, ACP_APP_ID_ALARM)
        assert_equals(msg.type, ACP_MSG_TYPE_ALARM_KA_REPLY)
        assert_equals(msg.vehicle_desc.imei, "0490154100837810")
        assert_equals(msg.vehicle_desc.iccid, "08923440000000000003")
        # pass the test
        bench.test_passed()

    # after receiving a message on which is_alarm_ka_reply returns true, call
    # the check_alarm_ka_reply function
    # or in other words, call check_alarm_ka_reply when we receive an
    # alarm_ka_reply.
    autoreply[is_alarm_ka_reply] = check_alarm_ka_reply
    # We can also write the followin, which has the same effect and is much more
    # concise:
    #autoreply[AlarmKAReply] = check_alarm_ka_reply

    # wait for 10 seconds before failing the test
    yield 10
    bench.test_failed('No reply received in 10 seconds')
