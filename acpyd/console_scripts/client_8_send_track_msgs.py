# CAUTION: script requires to follow indentation precisely.

count = 0
engine_on = False

def script(conn):
    track_interval = 5
    coords = [-23.55,-46.63]

    def send_track(*args, **kwargs):
        global count
        global engine_on

        coords[0] = coords[0] + ((random() - 0.5)/1000.0)
        coords[1] = coords[1] + ((random() - 0.5)/1000.0)
        if count % 10 == 0:
            if not engine_on:
                breakdown_status = IEBreakdownStatus(source=[0,ACP_BKD_VEHICLE_ON,0])
                engine_on = True
            else:
                breakdown_status = IEBreakdownStatus(source=[0,ACP_BKD_VEHICLE_OFF,0])
                engine_on = False
        else:
            breakdown_status = None
        conn.send_msg(
            TrackPos(
                vehicle_desc=IEVehicleDesc(
                    iccid='18'
                ),
                timestamp=IETimestamp(
                    time=time()
                ),
                location=IELocation(
                    curr_gps=IEGPSRawData(coords=coords)
                ),
                breakdown_status=breakdown_status
            )
        )
        count += 1

    # send the first one upon connect
    track_delayed = bench.periodic_timer(track_interval, send_track)
    while conn.connected:
        yield
