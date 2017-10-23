# A script that waits for a TIV command to send it's stored positions.

# PARAMETER DEFINITIONS
# These variables must not be modified by the script

# Vehicle descriptor and version to use on every
# message sent to the TIV
vehicle_desc = IEVehicleDesc(iccid='89550311000178178150')
version = IEVersion()

# Number of seconds to wait before sending the first message.
time_for_first_message = 5

# Number of messages with location information that must be sent
# after the TIV sent a request for position history.
# * THIS NUMBER DOES NOT INCLUDE THE FIRST MESSAGE *
expected_positions = 5

# Base coordinates to use on location messages
coords = [-25.5382,-49.1996]

def script(conn):
    # wait for first message
    yield time_for_first_message

    # no messages must be received before sending first message
    assert_false(conn.messages)

    # send first message
    conn.send_msg(TrackPos(
        vehicle_desc=vehicle_desc,
        version=version,
        timestamp=IETimestamp(time=time()),
        location=IELocation(curr_gps=IEGPSRawData(coords=coords)),
    ))

    def handle_func_cmd(msg):
        if (msg.ctrl_func.entity_id == ACP_ENT_ID_POS_HISTORY and
            msg.func_cmd.cmd == ACP_FUNC_CMD_ENABLE):
            for i in range(expected_positions):
                # send position history as independent messages
                conn.send_msg(TrackPos(
                    vehicle_desc=vehicle_desc,
                    version=version,
                    timestamp=IETimestamp(time=time()),
                    location=IELocation(curr_gps=IEGPSRawData(coords=coords)),
                ))
            bench.test_passed('sent position history, test passed')


    # configure replies for TCU messages
    autoreply[FuncCmd] = handle_func_cmd

    # wait
    while conn.connected:
        yield
