
cdef extern from "e_log.h":
    int e_log_set_level(int level)

def setlevel(int level):
    e_log_set_level(level)
