# CAUTION: script requires to follow indentation precisely.

def script(conn):
    prov_interval = 0.2
    track_cmd_interval = 0.2

    bench.debug("Processing message")
    # connected, set periodic notifications
    provs = []
    def send_prov(*args, **kwargs):
        msg = ProvUpd(
                      version=IEVersion(
                          car_manufacturer=0x01,
                          tcu_manufacturer=0x20,
                          major_hard_rel=0x01,
                          major_soft_rel=0x01,
                      ),
                      target_app_id = 0x70,
                      appl_flg=0x0,
                      start_time=IETimestamp(time=time()),
                  )
        provs.append(msg)
        conn.send_msg(msg)
    prov_timer = bench.periodic_timer(prov_interval, send_prov)

    def send_cfg(*args, **kwargs):
        prov_reply_exp = True
        conn.send_msg(CfgUpd245())
    prov_timer = bench.periodic_timer(prov_interval, send_prov)

    def send_track_cmd(*args, **kwargs):
        track_reply_exp = True
        conn.send_msg(TrackCmd())
    track_cmd_timer = bench.periodic_timer(track_cmd_interval, send_track_cmd)

    while conn.connected:
        # wait for msg
        yield
        if not conn.connected:
            break
        reply = None
        msg = conn.pop_msg()
        if msg.app_id == ACP_APP_ID_PROVISIONING:
            if msg.type == ACP_MSG_TYPE_PROV_UPD:
                bench.warn("Received prov upd from client")
                bench.test_failed("Received provision update from client")
            elif msg.type == ACP_MSG_TYPE_PROV_REPLY:
                if provs:
                    provs.pop()
                else:
                    bench.test_failed("Received provision reply when it wasnt expected")
        elif msg.app_id == ACP_APP_ID_VEHICLE_TRACKING:
            if msg.type == ACP_MSG_TYPE_TRACK_REPLY:
                bench.test_failed("Received track reply from client")
            elif msg.type == ACP_MSG_TYPE_TRACK_CMD:
                bench.test_failed("Received track command from client")
            elif msg.type == ACP_MSG_TYPE_TRACK_POS:
                reply = TrackReply()
        elif msg.app_id == ACP_APP_ID_ALARM:
            if msg.type == ACP_MSG_TYPE_ALARM_REPLY:
                bench.test_failed("Received alarm reply from client")
            elif msg.type == ACP_MSG_TYPE_ALARM_NOTIF:
                reply = AlarmReply()
        if reply is not None:
            conn.send_msg(reply)
