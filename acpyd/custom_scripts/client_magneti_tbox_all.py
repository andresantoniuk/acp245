def script(conn):
    forced_int = 1
    honor_timing = False
    coords = [-23.55,-46.63]
    vehicle_desc = IEVehicleDesc(iccid='123456')
    version = IEVersion()

    location_enabled = False
    in_event = False

    # count how many messages have been sent of each type
    sent = {
        AlarmPos: 0,
        AlarmNotif: 0,
        TrackPos: 0,
    }

    # define some helper functions
    def check_cfg_query(*types):
        assert_true(conn.messages)
        msg = conn.pop_msg()

        assert_true(isinstance(msg, CfgUpd245), 'message is not a CfgUpd245')
        assert_equals(msg.appl_flg, ACP_MSG_PROV_NO_CHANGE)
        assert_equals(len(msg.tcu_data.items), len(types))
        assert_equals(tuple([x.type for x in msg.tcu_data.items]), types)

    def check_cfg_upd(*args):
        assert_true(conn.messages)
        msg = conn.pop_msg()

        assert_true(isinstance(msg, CfgUpd245), 'message is not a CfgUpd245')
        assert_equals(msg.appl_flg, ACP_MSG_PROV_CHANGE)
        assert_equals(len(msg.tcu_data.items), len(args))
        assert_equals(tuple([(x.type, x.data) for x in msg.tcu_data.items]), args)

    def send_cfg_reply(*args):
        items = []
        for type, data in args:
            items.append(IETCUDataErrorItem(
                type=type, data=data, error=IEError(code=0)))

        msg = CfgReply245(
            vehicle_desc=vehicle_desc,
            version=version,
            error=IETCUDataError(items=items)
        )
        conn.send_msg(msg)

    def check_func_cmd(entity_id, cmd):
        assert_true(conn.messages)
        msg = conn.pop_msg()

        assert_true(isinstance(msg, FuncCmd))
        assert_equals(msg.ctrl_func.entity_id, entity_id)
        assert_equals(msg.func_cmd.cmd, cmd)

    def send_func_status(entity_id, cmd):
        msg = FuncStatus(
            vehicle_desc=vehicle_desc,
            version=version,
            ctrl_func=IECtrlFunc(entity_id=entity_id),
            func_status=IEFuncCmd(cmd=cmd)
        )
        conn.send_msg(msg)

    def send_pos(cls, loc=False, on=False, off=False, source=None):
        coords[0] = coords[0] + ((random() - 0.5)/1000.0)
        coords[1] = coords[1] + ((random() - 0.5)/1000.0)

        breakdown_status = None
        if on:
            breakdown_status = IEBreakdownStatus(source=[0,ACP_BKD_VEHICLE_ON,0])
        if off:
            breakdown_status = IEBreakdownStatus(source=[0,ACP_BKD_VEHICLE_OFF,0])
        if source:
            breakdown_status = IEBreakdownStatus(source=source)
        location = None
        if loc:
            location = IELocation(curr_gps=IEGPSRawData(coords=coords))
        conn.send_msg(
            cls(
                vehicle_desc=vehicle_desc,
                timestamp=IETimestamp(
                    time=time()
                ),
                location=location,
                breakdown_status=breakdown_status
            )
        )
        # Increment sent counter
        sent[cls] = sent[cls] + 1

    # Define timers (to send keep alive and location messages)
    def send_keepalives():
        conn.send_msg(AlarmKA())
    ka_timer = bench.periodic_timer(3, send_keepalives)

    def send_pos_report():
        if in_event:
            send_pos(AlarmPos, loc=location_enabled)
        else:
            send_pos(TrackPos, loc=location_enabled)
    report_pos_timer = bench.periodic_timer(honor_timing and 10 or forced_int, send_pos_report, start=False)

    # 1. server starts and waits for a connection
    # 2. first message must be a theft alarm with the KEY ON indication (first  message sent by the HW after we turn it on)
    send_pos(AlarmNotif, on=True)

    report_pos_timer.start()    # start sending TrackPos messages

    # 3. the server will query (config update message #2) parameter 0x0047 and verify it is FALSE
    yield
    check_cfg_query(0x0047)
    send_cfg_reply((0x0047, (0x00,)))

    # 4. the server will query parameter 0x0046 and verify it is FALSE
    yield
    check_cfg_query(0x0046)
    send_cfg_reply((0x0046, (0x00,)))

    # 5. server will collect 10 tracking messages and verify they have NO location data
    while sent[TrackPos] < 10:
        yield 1

    # 6. the server will set parameter 0x0047 to TRUE
    if not conn.messages: yield
    check_cfg_upd((0x0047, (0x01,)))
    location_enabled = True         # start sending location info
    sent[TrackPos] = 0          # clear sent counter
    send_cfg_reply((0x0047, (0x01,)))

    # 7. server will collect 10 tracking messages and verify they have location data
    while sent[TrackPos] < 10:
        yield 1

    # 8. server will set tracking timer to zero: if a tracking message is received at *any moment*, the test will fail
    if not conn.messages: yield
    check_cfg_upd((0x0011, (0x00,)))
    report_pos_timer.cancel()       # stop sending location info
    send_cfg_reply((0x0011,  (0x00,)))

    # 9. server will configure: keep alive timer to 60 seconds
    yield
    check_cfg_upd((0x0051, (60,)))
    ka_timer.start(honor_timing and 60 or forced_int)       # restart keepalive timer, 60 sec interval
    send_cfg_reply((0x0051,  (60,)))

    # 10. server will wait until it receives a keep alive timer - if the timeout to receive the keep alive expires, the test will fail
    # conn.send_msg(AlarmKA())      # ka is already being sent by timer

    # 11. server will set the keep alive timer to zero
    yield
    check_cfg_upd((0x0051, (0,)))
    ka_timer.cancel()               # stop sending keepalives
    send_cfg_reply((0x0051,  (0,)))

    # 12. server will set tracking timer on event mode to 90 seconds and verify the reply
    yield
    check_cfg_upd((0x0019, (90,)))
    send_cfg_reply((0x0019, (90,)))

    # 13. server will set tracking time in event mode to 75 seconds and verify the reply
    yield
    check_cfg_upd((0x0019, (75,)))
    send_cfg_reply((0x0019, (75,)))

    # 14. server will set IP address to 174.143.242.177 AND parameter 0x0046 o FALSE and verify the reply
    yield
    check_cfg_upd((0x0064, (174,143,242,177)), (0x0046, (0,)))
    send_cfg_reply((0x0064, (174,143,242,177)), (0x0046, (0,)))

    # 15. server will send a BLOCK command and verify the ACK from the TBOX
    yield
    check_func_cmd(ACP_ENT_ID_VEHICLE_BLOCK, ACP_FUNC_CMD_ENABLE)
    in_event = True                 # switch to event mode
    sent[AlarmPos] = 0              # clear sent counter
    report_pos_timer.start(honor_timing and 75 or forced_int)      # send location every 75 secs.
    send_func_status(ACP_ENT_ID_VEHICLE_BLOCK, ACP_FUNC_CMD_ENABLE)

    # 16. server will verify that the next 10 messages are a 0B 03 (ALERT TRACKING)
    while sent[AlarmPos] < 10:
        yield 1

    # 17. server will send an UNBLOCK and verify the reply
    if not conn.messages: yield
    check_func_cmd(ACP_ENT_ID_VEHICLE_BLOCK, ACP_FUNC_CMD_DISABLE)
    in_event = False                # switch to normal mode
    report_pos_timer.cancel()       # stop sending location info
    send_func_status(ACP_ENT_ID_VEHICLE_BLOCK, ACP_FUNC_CMD_DISABLE)

    # 18. set tracking timer to 30 and verify the reply
    yield
    check_cfg_upd((0x0011, (30,)))
    sent[TrackPos] = 0              # clear sent counter
    report_pos_timer.start(honor_timing and 30 or forced_int)   # send location every 30 seconds
    send_cfg_reply((0x0011,  (30,)))

    # 19. server will verify that the next 10 messages are are all Tracking messages (0A)
    while sent[TrackPos] < 10:
        yield 1

    # 20. server will send a BLOCK command and verify the ACK from the TBOX
    if not conn.messages: yield
    check_func_cmd(ACP_ENT_ID_VEHICLE_BLOCK, ACP_FUNC_CMD_ENABLE)
    # cancel TrackPos, send Alarm Pos
    in_event = True             # switch to event mode
    sent[AlarmPos] = 0          # clear sent counter
    report_pos_timer.start(honor_timing and 75 or forced_int)   # send location every 75 secs.
    send_func_status(ACP_ENT_ID_VEHICLE_BLOCK, ACP_FUNC_CMD_ENABLE)

    # 21. server will verify that the next 10 messages are a 0B 03 (ALERT TRACKING)
    while sent[AlarmPos] < 10:
        yield 1

    # 22. server will set the parameter 0x01FF to FALSE to disable the event mode and verify the reply
    if not conn.messages: yield
    check_cfg_upd((0x01FF, (0,)))
    in_event = False            # switch to normal mode
    sent[TrackPos] = 0      # clear sent counter
    report_pos_timer.start(honor_timing and 30 or forced_int)   # send location every 30 secs.
    send_cfg_reply((0x01FF, (0,)))

    # 23. server will verify that the next 10 messages are all Tracking messages (0A)
    while sent[TrackPos] < 10:
        yield 1

    # 24. server waits for a theft alarm with source = KEY OFF
    send_pos(AlarmNotif, off=True)

    # 25. end script, close connection.
