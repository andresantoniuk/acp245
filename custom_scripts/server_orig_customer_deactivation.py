# Sends a deactivation message to the TCU.
#
# This script can be executed against the TCU emulator.

# PARAMETER DEFINITIONS
# These variables must not be modified by the script

# Seconds to wait for the first message
time_to_wait_for_first_message = 120

# Seconds to wait for deactivation reply.
time_for_reply = 20

def script(conn):
    # wait for first message
    yield time_to_wait_for_first_message

    assert_true(conn.messages, 'TCU did not sent a message')
    first_msg = conn.pop_first()

    # check that first message includes a verion element and ICCID
    assert_true(first_msg.is_valid_tcu_first_msg(),
               'TCU sent an invalid first message')

    # ignore the following messages, autoreplying them
    def handle_track_pos(msg):
        return TrackReply(version=version)
    autoreply[TrackPos] = handle_track_pos

    def handle_alarm_notif(msg):
        return AlarmReply(version=version)
    autoreply[AlarmNotif] = handle_alarm_notif

    def handle_alarm_pos(msg):
        return AlarmReply(version=version)
    autoreply[AlarmPos] = handle_alarm_pos

    def handle_alarm_ka(msg):
        return AlarmKAReply(vehicle_desc=vehicle_desc)
    autoreply[AlarmKA] = handle_alarm_ka

    msg = stdmsg['13.4.5.3']
    msg.vehicle_desc = first_msg.vehicle_desc

    bench.debug('sending deactivation')
    conn.send_msg(msg)

    bench.debug('waiting for deactivation reply')
    yield time_for_reply

    reply = conn.pop_first(CfgReply)
    assert_true(reply is not None)
    assert_equals(reply.appl_flg, ACP_MSG_PROV_DEACTIVATE)
    assert_equals(reply.error.code, 0)

    bench.debug('waiting for disconnection')
    yield
    # connection must be closed
    assert_false(conn.connected)
