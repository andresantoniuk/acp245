# CAUTION: script requires to follow indentation precisely.

# This example waits for an alarm KA and replies it using the
# autoreply feature
def script(conn):
    def process_alarm_ka(msg):
        bench.debug('MESSAGE IS %s' % msg)

        reply = AlarmKAReply()
        reply.vehicle_desc = msg.vehicle_desc
        return reply
        # alternatively, instead of returning,
        # you could pass the test doing:
        #conn.send_msg(reply)
        #bench.test_passed('OK')

    autoreply[AlarmKA] = process_alarm_ka
    yield
    # alternatively, you could pass the test
    # case when you receive the message and
    # just write an inifinite generic yield loop:
    #while True: yield
