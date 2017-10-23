def script(conn):
    recv = []
    def recv_ka():
        msg = conn.pop_first(AlarmKA)
        if msg is not None:
            recv.append(msg)
        if len(recv) == 3:
            bench.test_passed()
    def fail():
        bench.test_failed()
    bench.periodic_timer(1, recv_ka)
    bench.timer(10, fail)
    while True: yield
