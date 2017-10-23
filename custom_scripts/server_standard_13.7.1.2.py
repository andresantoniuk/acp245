# 13.7.1.2 Notification of "Ignition ON" with Vehicle Location Data (Data type 0x0047 = True) - Test Case
# Server side.
# Receive AlarmNotif

def script(conn):
    # wait for AlarmNotif (13.7.1.2)
    msg = conn.pop_first(AlarmNotif)
    while msg is None:
        if not conn.connected:
            fail("Didn't receive AlarmNotif before disconnection")
        yield
        msg = conn.pop_first(AlarmNotif)

    assert_true(msg.version is not None)
    # *** set to expected values ***
    #assert_equals(msg.version.tcu_manufacturer, 0x00)
    #assert_equals(msg.version.car_manufacturer, 0x00)
    #assert_equals(msg.version.major_hard_rel, 0x00)
    #assert_equals(msg.version.major_soft_rel, 0x00)

    assert_true(msg.location.curr_gps is not None)
    assert_true(msg.location.prev_gps is None)
    assert_true(msg.location.dead_reck is None)
    assert_true(msg.location.loc_delta is None)

    assert_true(msg.vehicle_desc is not None)
    # *** set to expected values ***
    #assert_equals(msg.vehicle_desc.iccid, '01234567890123456789')

    assert_equals(msg.breakdown_status.sensor, 0x01)

 
