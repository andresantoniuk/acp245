# CAUTION: script requires to follow indentation precisely.

def script(conn):
    # server received a connection, send message
    msg = CfgUpd245(
        header=Header(
            app_id=0x02,                            # Vehicle Alarm
            test=0x00,                              # Test ? 
            type=0x08,                              # Type
            version=0x02                             # Version
        ),
        target_app_id=0x1,
        tcu_data=IETCUData(
            items=[
                IETCUDataItem(
                    type=0x11,      # transmission interval (0x0011)
                    data=(0xFF,0xFF) 
                ),
                IETCUDataItem(
                    type=0x65,      # Server 1 port (for test purposes of having multiple parameters)
                    data=(0x42, 0x69) # =17001 in decimal
                ),
            ]
        ),
    ) # end of CfgUpd245    

    # send msg
    yield                                           #Wait for TCU to initate communication
    assert_true(conn.messages)
    rxmsg = conn.pop_msg(0)                         #Collect TCU/Vehicle data from first received message
    msg.vehicle_desc = IEVehicleDesc(
            vin = rxmsg.vehicle_desc.vin,           # Optional
            iccid = rxmsg.vehicle_desc.iccid,
            imei = rxmsg.vehicle_desc.imei          # Optional  
    )
    print "TCU data collected successfully"
    conn.send_msg(msg)

    # wait for Client Reply
    yield

    # check that there are available messages
    assert_true(conn.messages)
    # get reply message
    msg = conn.pop_msg(0)
    # check that is a configuration update 245 message
    assert_equals(msg.app_id, ACP_APP_ID_CONFIGURATION)
    assert_equals(msg.type, ACP_MSG_TYPE_CFG_REPLY_245)
    print "Configuration response received"
    print "Maintaining connection"
    while True: yield
