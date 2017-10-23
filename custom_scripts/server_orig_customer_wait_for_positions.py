# A script to wait or request stored positions from the TCU
#

# PARAMETER DEFINITIONS
# These variables must not be modified by the script

# Number of seconds to wait for the first message
# before failing the test because no message was received
time_for_first_message = 120

# Minimum number of messages with location information that
# should be received after the TCU reconnects.
# The number of messages will vary depending for how long the TCU was
# deactivated.
# * THIS NUMBER DOES NOT INCLUDE THE FIRST MESSAGE *
expected_positions = 5

# Number of seconds to wait to receive the expected_positions
# messages before sending a message to the TCU to explicitely request it
# to send them.
time_before_requesting_positions = 10

# Number of seconds to wait to receive the expected_positions
# messages.
time_for_expected_positions = 60

def script(conn):
    # wait for first message
    yield time_for_first_message

    assert_true(conn.messages)
    first_msg = conn.pop_first()

    # check that first message includes a verion element and ICCID
    assert_true(first_msg.is_valid_tcu_first_msg())

    # store the version and vehicle descriptor element for later use
    version = first_msg.version
    vehicle_desc = first_msg.vehicle_desc

    positions = []
    def handle_alarm_pos(msg):
        if msg.location:
            positions.append(msg)
        return AlarmReply(version=version)

    def handle_alarm_notif(msg):
        if msg.location:
            positions.append(msg)
        return AlarmReply(version=version)

    def handle_track_pos(msg):
        if msg.location:
            positions.append(msg)
        return TrackReply(version=version)

    # configure basic replies for TCU messages
    autoreply[TrackPos] = handle_track_pos
    autoreply[AlarmNotif] = handle_alarm_notif
    autoreply[AlarmPos] = handle_alarm_pos
    autoreply[AlarmKA] = AlarmKAReply()

    # register a timer to request the stored positions
    # if they have not been sent after time_before_requesting_positions
    # seconds
    def request():
        if len(positions) < expected_positions:
            bench.debug('requesting positions from TCU')
            # request them
            conn.send_msg(
                FuncCmd(
                    vehicle_desc=vehicle_desc,
                    version=version,
                    ctrl_func=IECtrlFunc(entity_id=ACP_ENT_ID_POS_HISTORY),
                    func_cmd=IEFuncCmd(cmd=ACP_FUNC_CMD_ENABLE),
                )
            )
        else:
            bench.test_passed()
    bench.timer(time_before_requesting_positions, request)

    # register a timer to check number of received positions
    # after time_for_expected_positions seconds
    def check():
        bench.debug("received %s positions", len(positions))

        if len(positions) < expected_positions:
            bench.test_failed('Did not receive positions in expected time')
        else:
            bench.test_passed()

    bench.timer(time_for_expected_positions, check)

    # wait
    while conn.connected:
        yield

    assert_equals(expected_positions,
                  len(positions),
                  'Did not receive positons before disconnect')
