# CAUTION: script requires to follow indentation precisely.

def script(conn):
    alarm_interval = 0.2
    alarm_prob = 0.8

    # connected, set periodic notifications
    def send_alarm(*args, **kwargs):
        if random() < alarm_prob:
            conn.send_msg(AlarmNotif(timestamp=IETimestamp(time=time())))
    alarm_delayed = bench.periodic_timer(alarm_interval, send_alarm)

    def send_track(*args, **kwargs):
        conn.send_msg(TrackPos(
            timestamp=IETimestamp(time=time())))
    # send the first one upon connect
    track_delayed = bench.periodic_timer(alarm_interval, send_track)

    cnt = 0
    while conn.connected:
        # wait for msg
        yield
        if not conn.connected:
            bench.test_failed('Disconnected by server')

        cnt += 1
        reply = None
        msg = conn.pop_msg()
        if msg.app_id == ACP_APP_ID_PROVISIONING:
            if msg.type == ACP_MSG_TYPE_PROV_UPD:
                reply = ProvReply()
            elif msg.type == ACP_MSG_TYPE_PROV_REPLY:
                bench.warn("Received prov reply from server")
        elif msg.app_id == ACP_APP_ID_VEHICLE_TRACKING:
            if msg.type == ACP_MSG_TYPE_TRACK_REPLY:
                pass # got track reply
            elif msg.type == ACP_MSG_TYPE_TRACK_CMD:
                reply = TrackPos(
                    timestamp=IETimestamp(time=time()))
        elif msg.app_id == ACP_APP_ID_ALARM:
            if msg.type == ACP_MSG_TYPE_ALARM_REPLY:
                pass # got alarm reply
            elif msg.type == ACP_MSG_TYPE_ALARM_NOTIF:
                bench.warn("Received alarm notif from server")
        if reply:
            conn.send_msg(reply)
        if cnt == 50:
            bench.test_passed("OK!")
