# A script that performs block/unblock cycles.
#
# This script can be executed against the TCU emulator.

# PARAMETER DEFINITIONS
# These variables must not be modified by the script

# Number of seconds to wait for the first message
# before failing the test because no message was received
time_to_wait_for_first_message = 30

# Number of block/unblock cycles to perform
block_unblock_cycles = 3

# Time between a block and unblock.
time_before_unblock = 30

# Seconds to wait for message replies.
wait_for_reply = 20

# Seconds to wait between each block/unblock cycle.
time_between_cycles = 30

# If True, query block status after blocking
query_after_blocking = True

def script(conn):
    # wait for first message
    yield time_to_wait_for_first_message

    # assert_true(conn.messages, 'TCU did not sent a message')
    # first_msg = conn.pop_first()

    # check that first message includes a verion element and ICCID
    # assert_true(first_msg.is_valid_tcu_first_msg(),
    #             'TCU sent an invalid first message')

    # store the version and vehicle descriptor element for later use
    # version = first_msg.version
    # vehicle_desc = first_msg.vehicle_desc

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

    # perform block/request/unblock cycles
    for i in range(block_unblock_cycles):

        bench.debug('sending block command')
        msg = FuncCmd(
            vehicle_desc = vehicle_desc,
            version = version,
            ctrl_func=IECtrlFunc(entity_id=ACP_ENT_ID_VEHICLE_BLOCK),
            func_cmd=IEFuncCmd(cmd=ACP_FUNC_CMD_ENABLE)
        )
        # expect a reply
        msg.set_reply_expected(True)

        # send message
        conn.send_msg(msg)

        bench.debug('waiting for block reply')
        yield wait_for_reply + 10

        # check that reply is OK
        reply = conn.pop_first(FuncStatus)
        assert_equals(reply.vehicle_desc.iccid, vehicle_desc.iccid)
        assert_equals(reply.ctrl_func.entity_id, ACP_ENT_ID_VEHICLE_BLOCK)
        assert_equals(reply.func_status.cmd, ACP_FUNC_STATE_ENABLED)
        assert_equals(reply.error.code, 0)

        if query_after_blocking:
            bench.debug('sending query command')
            # Query blocking status
            msg = FuncCmd(
                vehicle_desc = vehicle_desc,
                version = version,
                ctrl_func=IECtrlFunc(entity_id=ACP_ENT_ID_VEHICLE_BLOCK),
                func_cmd=IEFuncCmd(cmd=ACP_FUNC_CMD_REQUEST)
            )
            msg.set_reply_expected(True)
            conn.send_msg(msg)

            bench.debug('waiting for query reply')
            yield wait_for_reply + 10

            reply = conn.pop_first(FuncStatus)
            assert_equals(reply.vehicle_desc.iccid, vehicle_desc.iccid)
            assert_equals(reply.ctrl_func.entity_id, ACP_ENT_ID_VEHICLE_BLOCK)
            assert_equals(reply.func_status.cmd, ACP_FUNC_STATE_ENABLED)
            assert_equals(reply.error.code, 0)

        bench.debug('waiting some seconds before unblock')
        yield time_before_unblock

        bench.debug('sending unblock command')
        msg = FuncCmd(
            vehicle_desc = vehicle_desc,
            version = version,
            ctrl_func=IECtrlFunc(entity_id=ACP_ENT_ID_VEHICLE_BLOCK),
            func_cmd=IEFuncCmd(cmd=ACP_FUNC_CMD_DISABLE)
        )
        msg.set_reply_expected(True)
        conn.send_msg(msg)

        bench.debug('waiting for unblock reply')
        yield wait_for_reply + 10
        reply = conn.pop_first(FuncStatus)
        assert_equals(reply.vehicle_desc.iccid, vehicle_desc.iccid)
        assert_equals(reply.ctrl_func.entity_id, ACP_ENT_ID_VEHICLE_BLOCK)
        assert_equals(reply.func_status.cmd, ACP_FUNC_STATE_DISABLED)
        assert_equals(reply.error.code, 0)

        # wait some seconds before next cycle
        yield time_between_cycles
