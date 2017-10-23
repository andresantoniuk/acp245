# CAUTION: script requires to follow indentation precisely.

def script(conn):
    msg = FuncCmd(
        header=Header(
            app_id=0x06,                            # Remote Vehicle Function
            test=0x00,                              # Test ? 
            type=0x02,                              # Type
            version=0x02                            # Version
        ),
        version=IEVersion(
            car_manufacturer=0x22,                  # JLR ( 34 = 0x22 )
            tcu_manufacturer=0x8e,                  # OTHERS
            major_hard_rel=0x01,
            major_soft_rel=0x01
        ),
        ctrl_func=IECtrlFunc(
            entity_id=ACP_ENT_ID_VEHICLE_BLOCK,     # 0x??
            transmit_unit=ACP_EL_TIME_UNIT_SECOND,  # 0
            transmit_interval=60,                   # 0x3C
        ),
        func_cmd=IEFuncCmd(
            cmd=ACP_FUNC_CMD_DISABLE,                # 0x02
        ),
        #Commented to test ability to obtain ICCID/IMEI from first received message
        #vehicle_desc=IEVehicleDesc(
            #imei="354476050015472",                 # replace with real IMEI
            #iccid="89314404000055314257",            # replace with real ICCID
        #),
    )
    yield                                           #Wait for TCU to initate communication
    rxmsg = conn.pop_msg(0)                         #Read the connection establishment message and remove it from buffer
                                                    #Note that the number "0" indicates the number of the message in the order of the RX queue 
    msg.vehicle_desc=IEVehicleDesc(                 
        imei=rxmsg.vehicle_desc.imei,               #Obtain IMEI from received message
        iccid=rxmsg.vehicle_desc.iccid,             #Obtain ICCID from received message
    )
    print "TCU details obtained successfully"
    #assert_true(conn.messages)
    conn.send_msg(msg)
    # wait for remote vehicle function reply
    yield

    # check that there are available messages
    assert_true(conn.messages)
    msg = conn.pop_msg(0)   #Read the first unread message from buffer
    # check that is a remote vehicle function reply
    assert_equals(msg.app_id, ACP_APP_ID_REMOTE_VEHICLE_FUNCTION)
    #print "Function Status = ", msg.type
    print "Function Status = ", msg.func_status
    #assert_equals(msg.type, ACP_MSG_TYPE_FUNC_STATUS)

    # error element is present
    #assert_true(msg.error)
    # ACP_ERR_OK == 0 == OK
    if hasattr(msg, 'error'):
        print "Error Code = ", msg.error.code
    # assert_equals(msg.error.code, ACP_ERR_OK)
    print "Mobilization successful, maintaining connection"
    while True: yield