# 13.7.1.1 Theft Alarm Keep Alive - Test Case
# Server side.
# Receive AlarmKA

def script(conn):
    # wait for AlarmKA (13.7.1.1)
    msg = conn.pop_first(AlarmKA)
    while msg is None:
        if not conn.connected:
            fail("Didn't receive AlarmKA before disconnection")
        yield
        msg = conn.pop_first(AlarmKA)

    assert_true(msg.vehicle_desc is not None)
    # *** set to expected values ***
    #assert_equals(msg.vehicle_desc.iccid, '01234567890123456789')

 
