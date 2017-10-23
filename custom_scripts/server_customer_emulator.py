# A generic TIV (SO) emulator.
#
# It will only process and reply TCU messages.

# PARAMETER DEFINITIONS
# These variables must not be modified by the script

# if send_version == True, messages will contain the version element, otherwise
# the version element will not be included (or length will be 0)
send_version = True

# Number of seconds to wait for the first message
# before failing the test because no message was received
time_to_wait_for_first_message = 120

# if send_vehicle_desc_in_ka_reply == True, keepalive reply messages will
# contain a vehicle descriptor element
send_vehicle_desc_in_ka_reply = False

# if send_<msg>_reply == True, a reply to keepalive messages will be always
# sent. Otherwise, it will be sent only if the reply expected flag is set on the
# message header.
send_track_pos_reply = True
send_alarm_notif_reply = True
send_alarm_pos_reply = True
send_alarm_ka_reply = True

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

    def get_version():
        # Returns the version element if send_version is True, None otherwise

        if send_version:
            return version
        else:
            return None

    def handle_track_pos(msg):
        assert_equals(vehicle_desc.iccid, msg.vehicle_desc.iccid)
        if send_track_pos_reply or msg.is_reply_expected():
            return TrackReply(version=get_version())
    autoreply[TrackPos] = handle_track_pos

    def handle_alarm_notif(msg):
        assert_equals(vehicle_desc.iccid, msg.vehicle_desc.iccid)
        if send_alarm_notif_reply or msg.is_reply_expected():
            return AlarmReply(
                version=get_version(),
            )
    autoreply[AlarmNotif] = handle_alarm_notif

    def handle_alarm_pos(msg):
        assert_equals(vehicle_desc.iccid, msg.vehicle_desc.iccid)
        if send_alarm_pos_reply or msg.is_reply_expected():
            return AlarmReply(
                version=get_version(),
            )
    autoreply[AlarmPos] = handle_alarm_pos

    def handle_alarm_ka(msg):
        if msg.vehicle_desc and msg.vehicle_desc.iccid:
            # in alarm_ka, vehicle_desc is optional.
            assert_true(vehicle_desc.iccid, msg.vehicle_desc.iccid)
        if send_alarm_ka_reply or msg.is_reply_expected():
            if send_vehicle_desc_in_ka_reply:
                return AlarmKAReply(vehicle_desc=vehicle_desc)
            else:
                return AlarmKAReply()
    autoreply[AlarmKA] = handle_alarm_ka

    # wait until the peer disconnects.
    while conn.connected:
        yield
