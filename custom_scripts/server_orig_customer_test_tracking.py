# A script that performs a tracking timer configuration change and checks that
# the TCU transmission interval is as expected.
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

    bench.debug('configuring tracking timer and enabling location info')
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

    bench.debug('waiting for tracking positions')

    # stop autoreplying TrackPos.
    del autoreply[TrackPos]

    # wait for number_of_track_messages TrackPos messages.
    for i in range(number_of_track_messages):
        yield tracking_timer_value + 10 # use 10 seconds margin
        msg = conn.pop_first(TrackPos)

        assert_true(msg is not None, 'no message received in tracking interval')
        assert_true(msg.location is not None, 'no location info on message')
        assert_true(msg.location.curr_gps is not None, 'no GPS info on message')

        if msg.is_reply_expected():
            # send even messages with version element and odd messages
            # version element without
            if i % 2 == 0:
                conn.send_msg(TrackReply(version=version))
            else:
                conn.send_msg(TrackReply())

        bench.debug('received message %s', i+1)
