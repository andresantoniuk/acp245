def script(conn):
    # Number of extra seconds to wait for expected messages.
    # If the timer is 60, the script will wait for 60 + margin seconds
    # for a message before considering that the script is invalid
    margin = 10

    # Autoreply alarms keepalive
    autoreply[AlarmKA] = None   # None == ignore keepalive and don't send reply

    bench.debug("1. server starts and waits for a connection")
    yield                       # wait for first message

    bench.debug("2. first message must be a theft alarm with the KEY ON "\
                "indication (first message sent by the HW after we turn it "\
                "on)")
    assert_true(conn.messages)

    msg = conn.pop_msg()
    assert_true(isinstance(msg, AlarmNotif))
    assert_true(msg.breakdown_status.source[1] & ACP_BKD_VEHICLE_ON)

    # store vehicle descriptor
    vehicle_desc = msg.vehicle_desc
    # version will not be included
    version = None      # use 'version = msg.version'  if you want to send the
                        # version field on future messages

    # define some helper functions
    def send_cfg_query(type):
        ''' Sends a QUERY CfgUpd245 to the TBOX '''
        msg = CfgUpd245(
                vehicle_desc = vehicle_desc,
                version = version,
                appl_flg = ACP_MSG_PROV_NO_CHANGE,
                tcu_data = IETCUData(items=[IETCUDataItem(type=type)])
        )
        conn.send_msg(msg)

    def send_cfg_upd(*type_data):
        ''' Sends an UPDATE CfgUpd245 to the TBOX '''
        items = []
        for type, data in type_data:
            items.append(IETCUDataItem(type=type,data=data))
        msg = CfgUpd245(
                vehicle_desc = vehicle_desc,
                version = version,
                appl_flg = ACP_MSG_PROV_CHANGE,
                tcu_data = IETCUData(items=items)
        )
        conn.send_msg(msg)

    def send_func_cmd(entity_id, cmd):
        ''' Sends a FuncCmd to the TBOX '''
        msg = FuncCmd(
            vehicle_desc = vehicle_desc,
            version = version,
            ctrl_func=IECtrlFunc(entity_id=entity_id),
            func_cmd=IEFuncCmd(cmd=cmd)
        )
        conn.send_msg(msg)

    def check_cfg_reply(*type_data):
        ''' Checks a CfgUpd245 reply from the TBOX '''
        assert_true(conn.messages, 'no messages available')
        msg = conn.pop_msg()

        assert_true(isinstance(msg, CfgReply245),
                    'got %s and expected CfgReply245' % msg )
        assert_equals(len(msg.error.items), len(type_data))
        i = 0
        for type, data in type_data:
            assert_equals(msg.error.items[i].type, type)
            assert_equals(msg.error.items[i].data, data)
            assert_equals(msg.error.items[i].error.code, 0)
            i += 1

    def check_func_status(entity_id, cmd):
        ''' Checks a FuncStatus from the TBOX '''
        assert_true(conn.messages, 'no messages available')
        msg = conn.pop_msg()

        assert_true(isinstance(msg, FuncStatus),
                    'got %s and expected FuncStatus' % msg)
        assert_equals(msg.ctrl_func.entity_id, entity_id)
        assert_equals(msg.func_status.cmd, cmd)

    def check_pos(msg_cls, location=True):
        ''' Checks a location report message from the TBOX '''
        assert_true(conn.messages, 'no messages available')
        msg = conn.pop_msg()

        # check that the type of location message is as expected
        assert_true(isinstance(msg, msg_cls),
                   'got %s and expected %s' % (msg, msg_cls))

        # check if location info was expected
        if location:
            assert_true(msg.location.curr_gps is not None,
                       'message did not include location information')
        else:
            assert_true(msg.location.curr_gps is None,
                       'message included location information')


    bench.debug("3. the server will query (config update message #2) "\
                "parameter 0x0047 and verify it is FALSE")
    send_cfg_query(0x0047)
    yield 30                        # wait up to 30 seconds for reply
    check_cfg_reply((0x0047, (0x00,)))

    autoreply[TrackPos] = None   # None == ignore TrackPos and don't send reply

    bench.debug("4. the server will query parameter 0x0046 and verify it is "\
                "FALSE")
    send_cfg_query(0x0046)
    yield 30
    check_cfg_reply((0x0046, (0x00,)))

    bench.debug("5. server will collect 10 tracking messages and verify they "\
                "have NO location data")
    del autoreply[TrackPos]         # Stop ignoring TrackPos
    for i in range(10):
        yield 60 + margin           # wait up to 60 seconds for the message
        bench.debug("5... got message %s" % i)
        check_pos(TrackPos, location=False)

    autoreply[TrackPos] = None   # None == ignore TrackPos and don't send reply

    bench.debug("6. the server will set parameter 0x0047 to TRUE")
    send_cfg_upd((0x0047, (0x01,)))
    yield 30
    check_cfg_reply((0x0047, (0x01,)))

    bench.debug("7. server will collect 10 tracking messages and verify they "\
                "have location data")
    del autoreply[TrackPos]         # Stop ignoring TrackPos
    for i in range(10):
        yield 60 + margin
        bench.debug("7... got message %s" % i)
        check_pos(TrackPos, location=True)

    bench.debug("8. server will set tracking timer to zero: if a tracking "\
                "message is received at *any moment*, the test will fail")
    send_cfg_upd((0x0011, (0x00,)))
    yield 30
    check_cfg_reply((0x0011,  (0x00,)))

    bench.debug("9. server will configure: keep alive timer to 60 seconds")
    send_cfg_upd((0x0051, (60,)))
    yield 30
    check_cfg_reply((0x0051,  (60,)))

    bench.debug("10. server will wait until it receives a keep alive timer - "\
                "if the timeout to receive the keep alive expires, the test "\
                "will fail")
    del autoreply[AlarmKA]      # stop ignoring Alarm keepalives
    yield 60 + margin           # wait upt to 60 seconds for keepalive
    assert_true(conn.messages, 'No messages received in 60 seconds')
    msg = conn.pop_msg()
    assert_true(isinstance(msg, AlarmKA),
               'got %s and expected AlarmKA' % msg)

    bench.debug("11. server will set the keep alive timer to zero")
    send_cfg_upd((0x0051, (0,)))
    yield 30
    check_cfg_reply((0x0051, (0,)))

    bench.debug("12. server will set tracking timer on event mode to 90 "\
                "seconds and verify the reply")
    send_cfg_upd((0x0019, (90,)))
    yield 30
    check_cfg_reply((0x0019, (90,)))

    bench.debug("13. server will set tracking time in event mode to 75 "\
                "seconds and verify the reply")
    send_cfg_upd((0x0019, (75,)))
    yield 30
    check_cfg_reply((0x0019, (75,)))

    bench.debug("14. server will set IP address to 174.143.242.177 AND "\
                "parameter 0x0046 to FALSE and verify the reply")
    send_cfg_upd((0x0064, (174,143,242,177,)), (0x0046, (0,)))
    yield 30
    check_cfg_reply((0x0064, (174,143,242,177)), (0x0046, (0,)))

    bench.debug("15. server will send a BLOCK command and verify the ACK from "\
                "the TBOX")
    send_func_cmd(ACP_ENT_ID_VEHICLE_BLOCK, ACP_FUNC_CMD_ENABLE)
    yield 30
    check_func_status(ACP_ENT_ID_VEHICLE_BLOCK, ACP_FUNC_CMD_ENABLE)

    bench.debug("16. server will verify that the next 10 messages are a 0B 03 "\
                "(ALARM TRACKING)")
    for i in range(10):
        yield 75 + margin       # wait up to 75 seconds for position
        bench.debug("16... got message %s" % i)
        check_pos(AlarmPos, location=True)

    bench.debug("17. server will send an UNBLOCK")
    send_func_cmd(ACP_ENT_ID_VEHICLE_BLOCK, ACP_FUNC_CMD_DISABLE)
    yield 30
    check_func_status(ACP_ENT_ID_VEHICLE_BLOCK, ACP_FUNC_CMD_DISABLE)

    bench.debug("18. set tracking timer to 30 and verify the reply")
    send_cfg_upd((0x0011, (30,)))
    yield 30
    check_cfg_reply((0x0011,  (30,)))

    bench.debug("19. server will verify that the next 10 messages are are "\
                "all Tracking messages (0A) received every 30 seconds")
    for i in range(10):
        yield 30 + margin
        bench.debug("19... got message %s" % i)
        check_pos(TrackPos, location=True)

    bench.debug("20. server will send a BLOCK command and verify the ACK from "\
                "the TBOX")
    send_func_cmd(ACP_ENT_ID_VEHICLE_BLOCK, ACP_FUNC_CMD_ENABLE)
    yield 30
    check_func_status(ACP_ENT_ID_VEHICLE_BLOCK, ACP_FUNC_CMD_ENABLE)

    bench.debug("21. server will verify that the next 10 messages are a 0B 03 "\
                "(ALARM TRACKING)""")
    for i in range(10):
        yield 75 + margin
        bench.debug("21... got message %s" % i)
        check_pos(AlarmPos, location=True)

    bench.debug("22. server will set the parameter 0x01FF to FALSE to disable "\
                "the event mode and verify the reply")
    send_cfg_upd((0x01FF, (0,)))
    yield 30
    check_cfg_reply((0x01FF, (0,)))

    bench.debug("23. server will verify that the next 10 messages are all "\
                "Tracking messages (0A)")
    for i in range(10):
        yield 30 + margin
        bench.debug("23... got message %s" % i)
        check_pos(TrackPos, location=True)

    autoreply[TrackPos] = None   # None == ignore TrackPos and don't send reply

    bench.debug("24. server waits for a theft alarm with source = KEY OFF")
    msg = conn.pop_first(AlarmNotif)
    while msg is None:
        yield
        bench.debug("24... got message")
        msg = conn.pop_first(AlarmNotif)    # pop AlarmNotif, ignore others
    # check the flag is set
    assert_true(msg.breakdown_status.source[1] & ACP_BKD_VEHICLE_OFF)

    bench.debug("25. done, close connection and wait for reconnection.")
