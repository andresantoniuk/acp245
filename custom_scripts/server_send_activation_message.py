# 13.5.1 Blocking request: no version element (shortest message) - Test Case
# Server side.
# Send FuncCmd and receive FuncStatus as reply.

def script(conn):
    while True: yield