# 13.7.1.5 Theft Alarm in event mode Vehicle Position Message without Vehicle Location Data (Data type 0x0047 = False) - Test Case
# Server side.
# Receive AlarmPos

def script(conn):
    # wait for AlarmPos (13.7.1.5)
    msg = conn.pop_first(AlarmPos)
    while msg is None:
        if not conn.connected:
            fail("Didn't receive AlarmPos before disconnection")
        yield
        msg = conn.pop_first(AlarmPos)

    assert_true(msg.version is not None)
    # *** set to expected values ***
    #assert_equals(msg.version.tcu_manufacturer, 0x00)
    #assert_equals(msg.version.car_manufacturer, 0x00)
    #assert_equals(msg.version.major_hard_rel, 0x00)
    #assert_equals(msg.version.major_soft_rel, 0x00)

    assert_true(msg.location.curr_gps is None)
    assert_true(msg.location.prev_gps is None)
    assert_true(msg.location.dead_reck is None)
    assert_true(msg.location.loc_delta is None)

    assert_true(msg.vehicle_desc is not None)
    # *** set to expected values ***
    #assert_equals(msg.vehicle_desc.iccid, '01234567890123456789')

    assert_equals(msg.breakdown_status.sensor, 0x01)

 
