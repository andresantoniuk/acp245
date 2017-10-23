# A script that performs activation/deactivation of tracking services.
#
# This script can be executed against the TCU emulator.

# PARAMETER DEFINITIONS
# These variables must not be modified by the script

# Number of seconds to wait for the first message
# before failing the test because no message was received
time_to_wait_for_first_message = 10

# Number of tracking messages to wait for.
number_of_track_messages = 10

# Value of the tracking timer to use (0 to 255)
tracking_timer_value = 20

def script(conn):
    # wait for first message
    yield time_to_wait_for_first_message

    assert_true(conn.messages, 'TCU did not sent a message')
    first_msg = conn.pop_first()

    # check that first message includes a verion element and ICCID
    assert_true(first_msg.is_valid_tcu_first_msg(),
                'TCU sent an invalid first message')

    # store the version and vehicle descriptor element for later use
    version = first_msg.version
    vehicle_desc = first_msg.vehicle_desc

    def handle_track_pos(msg):
        return TrackReply(version=version)
    autoreply[TrackPos] = handle_track_pos

    def handle_alarm_notif(msg):
        return AlarmReply(version=version)
    autoreply[AlarmNotif] = handle_alarm_notif

    def handle_alarm_pos(msg):
        bench.warn('Test designed to work in normal mode, not in event mode')
        return AlarmReply(version=version)
    autoreply[AlarmPos] = handle_alarm_pos

    def handle_alarm_ka(msg):
        return AlarmKAReply(vehicle_desc=vehicle_desc)
    autoreply[AlarmKA] = handle_alarm_ka

    bench.debug('enable location information set timer to %s seconds',
                tracking_timer_value)
    msg = CfgUpd245(
            vehicle_desc=vehicle_desc,
            version=version,
            appl_flg=ACP_MSG_PROV_CHANGE,
            tcu_data=IETCUData(
                items=[
                    # set tracking timer to
                    IETCUDataItem(
                        type=ACP_PAR_TRACKING_TIMER,
                        data=(0x00, tracking_timer_value)
                    ),
                    # enable location data in case it's disabled
                    IETCUDataItem(
                        type=ACP_PAR_LOCATION_ENABLED,
                        data=(0x01,)
                    )
                ]
            )
    )
    # ignore the replies
    autoreply[CfgReply] = None
    autoreply[CfgReply245] = None

    conn.send_msg(msg)

    # wait some seconds
    yield 10

    bench.debug('configure to 0 seconds (disable tracking)')
    msg = CfgUpd245(
            vehicle_desc=vehicle_desc,
            version=version,
            appl_flg=ACP_MSG_PROV_CHANGE,
            tcu_data=IETCUData(
                items=[
                    # set tracking timer to 0 seconds (disable)
                    IETCUDataItem(
                        type=ACP_PAR_TRACKING_TIMER,
                        data=(0x00, 0x00)
                    ),
                ]
            )
    )
    conn.send_msg(msg)

    def fail_if_track_pos(msg):
        fail("TrackPos received, test failed")
    # fail the test case if a TrackPos is received
    autoreply[TrackPos] = fail_if_track_pos

    bench.debug('no tracking position must be received during 30 seconds.')
    yield 30 # wait 30 seconds to see if a track pos is received

    # stop ignoring tracking positions
    del autoreply[TrackPos]

    # enable tracking every 20 seconds but disable location information
    bench.debug('disable location information and set timer to 20 secs')
    msg = CfgUpd245(
            vehicle_desc=vehicle_desc,
            version=version,
            appl_flg=ACP_MSG_PROV_CHANGE,
            tcu_data=IETCUData(
                items=[
                    # set tracking timer to 20 seconds
                    IETCUDataItem(
                        type=ACP_PAR_TRACKING_TIMER,
                        data=(0x00, tracking_timer_value)
                    ),
                    # disable location data
                    IETCUDataItem(
                        type=ACP_PAR_LOCATION_ENABLED,
                        data=(0x00,)
                    )
                ]
            )
    )
    conn.send_msg(msg)

    bench.debug('checking that messages dont include location information')
    # wait for number_of_track_messages TrackPos messages.
    for i in range(number_of_track_messages):
        yield tracking_timer_value + 10 # use 10 seconds margin
        msg = conn.pop_first(TrackPos)

        assert_true(msg is not None,
                    'no message received in tracking interval')
        # location information must NOT be present
        assert_true(msg.location is None or msg.location.curr_gps is None,
                    'location info on message')

        if msg.is_reply_expected():
            # send even messages with version element and odd messages
            # version element without
            if i % 2 == 0:
                conn.send_msg(TrackReply(version=version))
            else:
                conn.send_msg(TrackReply())

        bench.debug('received message %s', i+1)
