# This is a generic TCU emulator.
#
# It will remember configuration changes during the same connection, but state
# will be reseted on every new connection.

# PARAMETER DEFINITIONS
# These variables must not be modified by the script

# Define the vehicle descriptor and version to use on every
# message sent to the TIV
vehicle_desc = IEVehicleDesc(iccid='123456')
version = IEVersion()

# if send_version == True, messages will contain the version element, otherwise
# the version element will not be included (or length will be 0)
send_version = True

# if send_vehicle_desc_in_ka == True, keepalive messages will contain a
# vehicle descriptor element
send_vehicle_desc_in_ka = False

# if start_in_event == True, TCU starts on event mode
start_in_event = False

# starting coordinates for TCU
start_coords = [-23.55,-46.63]

# Number of seconds to wait before sending the first message
# Used to verify that the TIV does not close the connection
# for at least this number of seconds, in case the TCU is
# taking some time to send the first message.
time_to_wait_before_first_msg = 4

# time to run the script (in seconds)
time_to_run = 300

# if True, the TCU will mark it's TrackPos, AlarmKA and AlarmNotif as expecting
# replies
expect_reply = True

def script(conn):
    # start at start_coords
    coords = start_coords

    # start in event?
    in_event = start_in_event

    # * Initial TCU parameters *
    # parameters not defined here will not be known by this TCU emulator,
    # therefore if a message is received from the TIV querying/updating
    # parameters not defined here, an error will be sent as a reply.
    parameters = {
        ACP_PAR_TRACKING_TIMER:             (0x00,15), # timer = 15 seconds
        ACP_PAR_TRACKING_EVENT_TIMER:       (0x00,5),  # timer = 5 seconds
        ACP_PAR_ACTIVATION_ENABLED:         (0x01,),    # activation enabled = true
        ACP_PAR_LOCATION_ENABLED:           (0x00,),    # location enabled = false
        ACP_PAR_APN:                        'apn.customer.test',
        ACP_PAR_LOGIN:                      'customer',
        ACP_PAR_PASSWORD:                   'customer',
        ACP_PAR_SERVER_IP_1:                (4,4,4,2),       # ip = 4.4.4.2
        ACP_PAR_SERVER_PORT_1:              (0x27,0x12),     # port = 10000
        ACP_PAR_KEEPALIVE_TIMER:            (0x00,10), # timer = 10 seconds
    }
    # -> alternative: do not include ACP_PAR_TRACKING_EVENT_TIMER or
    # ACP_PAR_KEEPALIVE_TIMER, which are optional parameters, so that an error
    # reply is sent when the value is configured.

    # * Status of each entity managed by the TCU *
    # entities not defined here will not be known by this TCU emulator,
    # therefore if a message is received from the TIV trying to control entities
    # not defined here, an error will be sent as a reply.
    entity_status = {
        ACP_ENT_ID_VEHICLE_BLOCK: ACP_FUNC_STATE_DISABLED,    # TCU not blocked
        ACP_ENT_ID_POS_HISTORY: 0x0,                      # value is irrelevant
    }

    # list to record position history, in case it's queried.
    history = []

    # * HELPER FUNCTIONS *
    # The following functions are defined to write less code on the rest of the
    # script.
    def get_location():
        # Returns a location element if location enabled (parameter 0x0047==1),
        # None otherwise
        if parameters[ACP_PAR_LOCATION_ENABLED] != (0x00,):
            return IELocation(curr_gps=IEGPSRawData(coords=coords))
        else:
            return None

    # save 10 positions of initial history.
    for i in range(10):
        history.append((time(), get_location()))

    def get_version():
        # Returns the version element if send_version is True, None otherwise

        if send_version:
            return version
        else:
            return None

    def get_timer_interval():
        # Returns the number of seconds that must pass before sending a new
        # location to the TIV.

        if in_event and (ACP_PAR_TRACKING_EVENT_TIMER in parameters):
            return as_int(parameters[ACP_PAR_TRACKING_EVENT_TIMER])
        else:
            return as_int(parameters[ACP_PAR_TRACKING_TIMER])

    def get_keepalive_interval():
        # Returns the number of seconds that must pass before sending a new
        # location to the TIV

        if ACP_PAR_KEEPALIVE_TIMER in parameters:
            return as_int(parameters[ACP_PAR_KEEPALIVE_TIMER])
        else:
            return 60

    def send_position_history():
        # Sends all the positions stored on the history list

        for seconds, location in history:
            # send position history as independent messages
            reply = TrackPos(
                vehicle_desc=vehicle_desc,
                version=get_version(),
                timestamp=IETimestamp(time=seconds),
                location=location,
            )
            if expect_reply: reply.expect_reply()
            conn.send_msg(reply)

    def handle_func_cmd(msg):
        # A function to handle Function Command messages received from
        # the TIV.
        # This function modifies or queries the entity_status of this
        # TCU emulator, based on the content of the message.

        assert_equals(vehicle_desc.iccid, msg.vehicle_desc.iccid)

        entity_id = msg.ctrl_func.entity_id
        func_status = msg.func_cmd
        error = None

        if entity_id not in entity_status:
            # Unknown entity id, send error response
            error = IEError(code=ACP_ERR_GENERAL)
        elif msg.func_cmd.cmd == ACP_FUNC_CMD_ENABLE:
            if msg.ctrl_func.entity_id == ACP_ENT_ID_POS_HISTORY:
                # send position history in 3 seconds
                bench.timer(3, send_position_history)
            entity_status[entity_id] = ACP_FUNC_STATE_ENABLED
        elif msg.func_cmd.cmd == ACP_FUNC_CMD_DISABLE:
            if entity_id == ACP_ENT_ID_VEHICLE_TRACK:
                # comply with section 14.10 of the specification.
                error = IEError(code=ACP_ERR_GENERAL)
            else:
                entity_status[entity_id] = ACP_FUNC_STATE_DISABLED
        elif msg.func_cmd.cmd == ACP_FUNC_CMD_REQUEST:
            # return the status stored for that entity_id
            func_status = IEFuncCmd(cmd=entity_status[entity_id])
        else:
            # Dont know how to process other function commands, send error response
            error = IEError(code=ACP_ERR_GENERAL)

        # send reply
        return FuncStatus(
            version=get_version(),
            vehicle_desc=vehicle_desc,
            ctrl_func=msg.ctrl_func,
            func_status=func_status,
            error=error,
        )

    def handle_prov_upd(msg):
        # A function to handle Provision Update messages received from
        # the TIV

        assert_equals(vehicle_desc.iccid, msg.vehicle_desc.iccid)

        error = None

        # check if the provisioned application ID is a valid one.
        if target_app_id in (1,2,6,10,11):
            if msg.appl_flg == ACP_MSG_PROV_DEACTIVATE:
                bench.debug('deactivation attempt, sending error reply')
                # ACP245 doesn't allow deactivation of applications, send an
                # error message.
                error = IEError(code=ACP_ERR_GENERAL)
            else:
                bench.debug('activation attempt, sending already provisioned')
                # every ACP245 application is provisioned by default, return
                # that the application is already provisioned.
                status = ACP_MSG_PROV_STATUS_ALREADY_PROV
        else:
            # invalid application ID, send error message
            error = IEError(code=ACP_ERR_PROV_UNABLE_TO_PROC)

        if error:
            # an error will be sent, status must be set to this value
            status = ACP_MSG_PROV_STATUS_SEE_ERROR

        # send reply
        return ProvReply(
            vehicle_desc=vehicle_desc,
            version=get_version(),
            target_app_id=msg.target_app_id,
            appl_flg=msg.appl_flg,
            tcu_resp=ACP_MSG_PROV_TCU_RESP_TO_UPD,
            status=status,
        )

    def handle_cfg_upd_245(msg):
        # A function to handle Configuration Update 245 messages received from
        # the TIV

        assert_equals(vehicle_desc.iccid, msg.vehicle_desc.iccid)

        reply_items = [] # A list to store configuration replies

        # check if message includes configuration data
        if msg.tcu_data:

            # check if message is an update (change)
            if msg.appl_flg == ACP_MSG_PROV_CHANGE:
                for item in msg.tcu_data.items:

                    # check if parameter is known by this TCU
                    if item.type in parameters:
                        bench.debug('changing configuration 0x%X: %s',
                                   item.type, item.data)
                        # known parameter, no error

                        # add reply to reply items
                        reply_items.append(
                            IETCUDataErrorItem(
                                type=item.type, data=item.data,
                                error=IEError(code=0) # 0 means no error
                            )
                        )

                        # update parameters
                        parameters[item.type] = item.data

                        # if timer changed, restart timer with new interval
                        if item.type in (ACP_PAR_TRACKING_TIMER,
                                         ACP_PAR_TRACKING_EVENT_TIMER):
                            bench.debug('canceling timer')
                            report_pos_timer.cancel()

                            if item.data != (0x0,0x0):
                                bench.debug('starting report position timer')
                                report_pos_timer.start(interval=get_timer_interval())
                            #else, interval == 0, so do not start the report_pos_timer

                        # if keepalive timer changed, restart timer with new interval
                        elif item.type == ACP_PAR_KEEPALIVE_TIMER:
                            bench.debug('canceling keepalive timer')
                            keepalive_timer.cancel()

                            if item.data != (0x0,0x0):
                                bench.debug('starting keepalive position timer')
                                keepalive_timer.start(interval=get_keepalive_interval())
                            #else, interval == 0, so do not start the keepalive_timer
                    else:
                        # unknown parameter, send error
                        bench.warn('unknown parameter 0x%X, sending error',
                                   item.type)

                        # add reply to reply items
                        reply_items.append(
                            IETCUDataErrorItem(
                                type=item.type, data='',
                                error=IEError(code=ACP_ERR_GENERAL)
                            )
                        )

            # check if message is a query (no change)
            elif msg.appl_flg == ACP_MSG_PROV_NO_CHANGE:
                for item in msg.tcu_data.items:
                    if item.type in parameters:
                        bench.debug('querying parameter 0x%X', item.type)
                        # known parameter, no error
                        reply_items.append(
                            IETCUDataErrorItem(
                                type=item.type, data=item.data,
                                error=IEError(code=0)   # 0 means no error
                            )
                        )
                    else:
                        bench.warn('unknown parameter 0x%X', item.type)
                        # unknown parameter, send error
                        reply_items.append(
                            IETCUDataErrorItem(
                                type=item.type, data='',
                                error=IEError(code=ACP_ERR_GENERAL)
                            )
                        )

        # check if we are sending replies for more than 1 parameter
        if len(reply_items) > 1:
            # send a reply for multiple parameters
            return CfgReply245(
                vehicle_desc=vehicle_desc,
                version=get_version(),
                appl_flg=msg.appl_flg,
                error=IETCUDataError(items=reply_items)
            )
        elif len(reply_items) == 1:
            # send a reply for single parameter
            return CfgReply(
                vehicle_desc=vehicle_desc,
                version=get_version(),
                appl_flg=msg.appl_flg,
                error=reply_items[0].error
            )
        else:
            # no parameter, send an empty reply
            return CfgReply(
                vehicle_desc=vehicle_desc,
                version=get_version(),
                appl_flg=msg.appl_flg,
            )
        # -> alternative: send always a reply for multiple parameters, or send
        # one reply for single parameters for each configured parameter.

    def handle_activation(msg):
        # only valid activation message received through GPRS is a
        # deactivation message.
        bench.debug('Deactivating and sending reply')
        assert_equals(vehicle_desc.iccid, msg.vehicle_desc.iccid)
        assert_false(msg.ctrl_byte &
                    ACP_MSG_CFG_CTRL_VALUE_ACTIVATE) # activation is not set
        # send reply and disconnect
        conn.send_msg(CfgReply(
            version=get_version(),
            vehicle_desc=vehicle_desc,
            appl_flg=ACP_MSG_PROV_DEACTIVATE
        ))
        conn.close()

    def send_pos_report():
        # A function to send position reports to the TIV

        # always increment the current coordinates on each pos report
        coords[0] = coords[0] + 0.0001
        coords[1] = coords[1] + 0.0001
        seconds = time()
        location = get_location()

        # remember sent position in history
        history.append((seconds, location))

        if in_event:
            # in event, send Alarm Positions
            msg = AlarmPos(
                vehicle_desc=vehicle_desc,
                version=get_version(),
                timestamp=IETimestamp(time=seconds),
                location=location,
            )
        else:
            # normal mode, send Track Positions
            msg = TrackPos(
                vehicle_desc=vehicle_desc,
                version=get_version(),
                timestamp=IETimestamp(time=seconds),
                location=location,
            )

        # send the message
        if expect_reply: msg.expect_reply()
        conn.send_msg(msg)
        # -> alternative: require a reply

    def send_keepalives():
        # A function to send keepalives to the TIV.

        if send_vehicle_desc_in_ka:
            msg = AlarmKA(vehicle_desc=vehicle_desc)
        else:
            msg = AlarmKA()
        if expect_reply: msg.expect_reply()
        conn.send_msg(msg)

    # a timer to send keep alives
    keepalive_timer = bench.periodic_timer(
        get_keepalive_interval(),
        send_keepalives
    )

    # a timer to send location messages to the TIV
    report_pos_timer = bench.periodic_timer(
        get_timer_interval(),
        send_pos_report,
    )

    # first message to sent to the TIV
    first_msg = TrackPos(
        vehicle_desc=vehicle_desc,
        version=version,
        timestamp=IETimestamp(time=time()),
        location=get_location(),
    )
    # -> alternative: send a different type of message

    # wait before sending first message (to see if TIV sents a message
    # before we send the first one).
    yield time_to_wait_before_first_msg
    assert_false(conn.messages, 'TIV must wait until TCU sends the first message')

    autoreply[CfgActivation] = handle_activation
    autoreply[CfgUpd245] = handle_cfg_upd_245
    autoreply[ProvUpd]   = handle_cfg_upd_245
    autoreply[FuncCmd]   = handle_func_cmd
    # ignore track replies from TIV
    autoreply[TrackReply] = None
    # ignore alarm replies from TIV
    autoreply[AlarmReply] = None
    # ignore alarm KA replies from TIV
    autoreply[AlarmKAReply] = None

    conn.send_msg(first_msg)
    # -> alternative: send message with reply_expected set to 1 and wait for reply

    report_pos_timer.start()    # start sending TrackPos messages
    keepalive_timer.start()     # start sending AlarmKA messages

    # wait up to time_to_run, or until the peer disconnects
    remaining = time_to_run
    while remaining > 0 and conn.connected:
        started = time()
        yield remaining
        remaining = remaining - (time() - started)
