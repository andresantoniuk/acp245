# CAUTION: script requires to follow indentation precisely.

def script(conn):
    def process_alarm_ka(msg):
        reply = AlarmKAReply()
        reply.vehicle_desc = msg.vehicle_desc
        return reply
    def process_track_pos(msg):
        reply = TrackPosReply()
        reply.vehicle_desc = msg.vehicle_desc
        return reply
    # server received a connection, send message
    msg = CfgUpd245(
        header=Header(
            app_id=0x02,                            # Configuration
            test=0x00,                              # Test ? 
            type=0x08,                              # Type
            version=0x02                            # Version
        ),
        target_app_id=0x00,
        appl_flg=0x00,                               # 0x00 = no change, request status 0x03 - change for this application

        vehicle_desc=IEVehicleDesc(
            vin="123456789012345",
            iccid="89314404000055312152"
            # do not include, IMEI, ICCID or Authentication Key
        ),
        tcu_data=IETCUData(
            items=[
                IETCUDataItem(
                    type=0x0047,      # Location Enabled
                    #data=(0x01, 0x2D) # 301  seconds (0x2D)
                ),
            ]
        )
    ) # end of CfgUpd245    

    # send msg
    conn.send_msg(msg)
    while True:
        # wait for Client Reply
        yield 20

        # check that there are available messages
        assert_true(conn.messages)
        # get first message
        msg = conn.pop_msg(0)
 
        autoreply[AlarmKA] = process_alarm_ka
        autoreply[TrackPos] = process_track_pos

        if msg.header.app_id is 2:
            # check that is a configuration update 245 message
            assert_equals(msg.type, ACP_MSG_TYPE_CFG_REPLY_245)
            bench.test_passed                 #breaks from the infinite message receive loop 
