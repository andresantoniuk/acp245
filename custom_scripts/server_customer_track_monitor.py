# CAUTION: script requires to follow indentation precisely.

def script(conn):
    def process_alarm_ka(msg):
        reply = AlarmKAReply()
        reply.vehicle_desc = msg.vehicle_desc
        return reply

    # wait for client vehicle position
    autoreply[AlarmKA] = process_alarm_ka
    while True:
        yield 301
        assert_true(conn.messages)
        # get first message
        msg = conn.pop_msg(0)
        if msg.type is 1:
            msg1 = AlarmReply(
                ##app_id = ACP_APP_ID_ALARM,
                ##type = ACP_MSG_TYPE_ALARM_REPLY,
                # Uncommenting previous lines causes the script to fail.
                version=IEVersion(
                    car_manufacturer = 0x06,# Car manufacturer is Jaguar (06)
                    tcu_manufacturer = 144, # TCU manufacturer is OTHERS (144)
                    # release numbers
                    major_hard_rel = 0x1,
                    major_soft_rel = 0x1
                ),
                confirmation = 5,
                transmit_unit = 0,
                ctrl_flg = 0x10,
                error = IEError(
                    code = 0x00
                )
            )
            msg1.header.version = 2
            bench.debug('Alarm Notification Reply generated')
            # send msg1
            conn.send_msg(msg1)
            bench.debug('Alarm Notification reply sent')
