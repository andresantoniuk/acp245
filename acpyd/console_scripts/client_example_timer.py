def script(conn):
    m = AlarmKA()
    sent = []
    def  send_ka():
        sent.append(m)
        conn.send_msg(m) # access to m and conn
    t1 = bench.timer(1, send_ka) # send ka
    t2 = bench.timer(2, send_ka) # send ka
    t3 = bench.timer(3, send_ka) # send ka
    while len(sent) != 3: yield
