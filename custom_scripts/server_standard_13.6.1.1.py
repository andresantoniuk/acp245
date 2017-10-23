# 13.6.1.1 Vehicle Position Message with Vehicle Location Data (Data type 0x0047 = True) - Test Case
# Server side.
# Receive TrackPos

def script(conn):
    # wait for TrackPos (13.6.1.1)
    msg = conn.pop_first(TrackPos)
    while msg is None:
        if not conn.connected:
            fail("Didn't receive TrackPos before disconnection")
        yield
        msg = conn.pop_first(TrackPos)

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

 
