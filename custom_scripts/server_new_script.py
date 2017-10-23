# ACP v1.2.2 - Minimum Message Set Compliance
# According to ACP 245 V 1.2.2_ 23_11_10_WITH_SMS.pdf


# VARIABLES

# sent message
var_ICCID = "89314404000055312293"

var_ITEM_TYPE = 0x11
# var_ITEM_DATA = ( 0x00, 0x3C ) # 60 seconds


# expected values

# Header
exp_HEADER_TEST     = 0 # test flag cleared
exp_HEADER_VERSION  = ACP_VER_1_2_2
exp_HEADER_MSG_CTRL = 0x00 # all flags cleared

# Version Element
exp_VERSION_CAR_MANUFACTURER = 34  # Land Rover
exp_VERSION_TCU_MANUFACTURER = 144 # Actia
exp_VERSION_MAJOR_HARD_REL   = 1
exp_VERSION_MAJOR_SOFT_REL   = 1

# Message Fields
exp_TARGET_APP_ID = 0 # no app
exp_APPL_FLG      = ACP_MSG_PROV_NO_CHANGE
exp_CTRL_FLG1     = ACP_MSG_PROV_VEHICLE_DESC_MASK
exp_STATUS        = ACP_MSG_PROV_STATUS_ALREADY_PROV
exp_TCU_RESP      = 0 # reserved

# TCU Data Error Element
exp_TCU_DATA_ERROR_ITEM_TYPE       = var_ITEM_TYPE
exp_TCU_DATA_ERROR_ITEM_DATA       = ( 0x00, 0x3C ) # 60 seconds
exp_TCU_DATA_ERROR_ITEM_ERROR_CODE = ACP_ERR_OK

# Vehicle Descriptor Element
exp_VEHICLE_DESC_ICCID = var_ICCID



# SCRIPT

def script (conn) :

    # PART 1 - creating message

    out_msg = CfgUpd245 (

        header = Header (
            test     = False,
            version  = ACP_VER_1_2_2,
            msg_ctrl = ACP_HDR_MSG_CTRL_RESP_EXP,
            msg_prio = 0
        ),

        # version - not present in this message

        # Message Fields
        target_app_id = 0,
        appl_flg      = ACP_MSG_PROV_NO_CHANGE,
        ctrl_flg1     = ACP_MSG_PROV_VEHICLE_DESC_MASK,
        # ctrl_flg2 - not present since ACP_MSG_PROV_ADDL_FLG_MASK is not set

        # start_time - not present in this message
        # end_time   - not present in this message
        # grace_time - not present in this message

        # tcu_desc   - not present in this message
        
        tcu_data = IETCUData (
                        items = [
                            IETCUDataItem (
                                type = var_ITEM_TYPE
#                                data = var_ITEM_DATA
                            )
                        ]
        ),

        vehicle_desc = IEVehicleDesc (
            iccid = var_ICCID
        ),

    ) # end of CfgUpd245 



    # PART 2 - sending message

    # wait for connection message
    yield
    
    # send CfgUpd245 message
    conn.send_msg ( out_msg )
    


    # PART 3 - verifying message

    # get first ProvReply message
    in_msg = conn.pop_first ( CfgReply245 )
    while in_msg is None :
        yield
        in_msg = conn.pop_first ( CfgReply245 )
      
    # Header verification
    assert_true   ( in_msg.header )
    # app_id already verified
    assert_equals ( in_msg.header.test,     exp_HEADER_TEST )
    # type already verified
    assert_equals ( in_msg.header.version,  exp_HEADER_VERSION )
    assert_equals ( in_msg.header.msg_ctrl, exp_HEADER_MSG_CTRL )
    assert_false  ( in_msg.header.msg_prio )

    # Version Element verification
    assert_true   ( in_msg.version )
    assert_equals ( in_msg.version.car_manufacturer, exp_VERSION_CAR_MANUFACTURER )
    assert_equals ( in_msg.version.tcu_manufacturer, exp_VERSION_TCU_MANUFACTURER )
    assert_equals ( in_msg.version.major_hard_rel,   exp_VERSION_MAJOR_HARD_REL )
    assert_equals ( in_msg.version.major_soft_rel,   exp_VERSION_MAJOR_SOFT_REL )

    # Message Fields verification
    assert_equals ( in_msg.target_app_id, exp_TARGET_APP_ID )
    assert_equals ( in_msg.appl_flg,      exp_APPL_FLG )    
    assert_equals ( in_msg.ctrl_flg1,     exp_CTRL_FLG1 )    
    assert_equals ( in_msg.status,        exp_STATUS )    
    assert_equals ( in_msg.tcu_resp,      exp_TCU_RESP )    

    # TCU Data Error element verification
    assert_true   ( in_msg.error )
    assert_equals ( in_msg.error.items[0].type,       exp_TCU_DATA_ERROR_ITEM_TYPE )
    assert_equals ( in_msg.error.items[0].data,       exp_TCU_DATA_ERROR_ITEM_DATA )
    assert_equals ( in_msg.error.items[0].error.code, exp_TCU_DATA_ERROR_ITEM_ERROR_CODE )

    # Vehicle Descriptor verification
    assert_true   ( in_msg.vehicle_desc )
    assert_false  ( in_msg.vehicle_desc.lang )
    assert_false  ( in_msg.vehicle_desc.model_year )
    assert_false  ( in_msg.vehicle_desc.vin )
    assert_false  ( in_msg.vehicle_desc.tcu_serial )
    assert_false  ( in_msg.vehicle_desc.license_plate )
    assert_false  ( in_msg.vehicle_desc.vehicle_color )
    assert_false  ( in_msg.vehicle_desc.vehicle_model )
    assert_false  ( in_msg.vehicle_desc.imei )
    assert_equals ( in_msg.vehicle_desc.iccid, exp_VEHICLE_DESC_ICCID )
    assert_false  ( in_msg.vehicle_desc.auth_key )

    
    
    # PART 4 - final result

    # test passed
    bench.test_passed ( "Configuration Reply Message #2 data as expected" )
    
    # close connection
    conn.close ()