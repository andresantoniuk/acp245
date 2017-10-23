# CAUTION: script requires to follow indentation precisely.

def script(conn):
    # server received a connection, send message
    msg = FuncCmd(
        header=Header(
            app_id=0x06,                            # Remote Vehicle Function Service
            test=0x00,                              # Test ? 
            #type=0x02,                              # Type
            version=0x02                            # Version
        ),
        version=IEVersion(
            car_manufacturer=0x20,# Car manufacturer is Effa Motors (30)
            tcu_manufacturer=130, # TCU manufacturer is JCI (130)
            # release numbers
            major_hard_rel=0x1,
            major_soft_rel=0x2
        ),
        ctrl_func=IECtrlFunc(
            entity_id=1,          # 1 = Vehicle Tracking
            transmit_interval=60,
            transmit_unit=0,
        ),
        func_cmd=IEFuncCmd(
            cmd=2                 # 2 = Start service, 3 = Terminate service
        ),
        vehicle_desc=IEVehicleDesc(
            vin="123456789012345",
            iccid="89314404000055312152"
        )
    ) # end of FuncCmd

    # send msg
    conn.send_msg(msg)

    # wait for client vehicle position
    while True: yield
