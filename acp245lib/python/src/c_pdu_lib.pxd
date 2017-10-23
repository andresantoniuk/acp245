cdef extern from "stdlib.h":
    ctypedef unsigned long size_t
    void free(void *ptr)
    void *malloc(size_t size)

cdef extern from "string.h":
    void *memset(void *s, int c, size_t n)
    char *strdup(char *s)

cdef extern from "time.h":
    ctypedef long time_t
    cdef struct tm:
        int tm_sec
        int tm_min
        int tm_hour
        int tm_mday
        int tm_mon
        int tm_year
        int tm_wday
        int tm_yday
        int tm_isdst

cdef extern from "e_port.h":
    ctypedef int e_ret
    ctypedef unsigned char u8
    ctypedef char bool
    ctypedef char ascii
    ctypedef unsigned short u16
    ctypedef short s16
    ctypedef unsigned int u32
    ctypedef int s32
    ctypedef unsigned char bool
    cdef enum:
        TRUE
        FALSE

cdef extern from "e_buff.h":
    ctypedef struct e_buff:
        pass
    void e_buff_wrap(e_buff *buff, u8 *data, u32 capacity)
    void e_buff_set_lim(e_buff *buff, u32 lim)
    e_ret e_buff_write(e_buff *buff, u8 b)
    u32 e_buff_read_remain(e_buff *buff)

cdef extern from "e_time.h":
    time_t e_time_timegm(tm *t)
