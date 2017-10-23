/*=============================================================================
        Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

        This software is furnished under a license and may be used and copied
        only in accordance with the terms of such license and with the
        inclusion of the above copyright notice. This software or any other
        copies thereof may not be provided or otherwise made available to any
        other person. No title to and ownership of the software is hereby
        transferred.
==============================================================================*/
#ifndef __e_buff_h_
#define __e_buff_h_

#include "e_port.h"

#ifdef __cplusplus
extern "C" {
#endif

#define E_BUFF_ERR          -1
#define E_BUFF_OVERFLOW     -2
#define E_BUFF_UNDERFLOW    -3

/**
 * @invariant e_buff_get_lim() >= 0
 * @invariant e_buff_get_pos() >= 0
 * @invariant e_buff_get_pos() <= e_buff_get_lim()
 * @invariant e_buff_get_lim() <= e_buff_capacity()
 */
typedef struct _e_buff e_buff;

/**
 * Private structure.
 *
 * Members must not be accessed outside e_buff module. The structure
 * is provided so that the e_buff can be allocated as an automatic
 * variable when dynamic memory is not available or convenient.
 */
struct _e_buff {
    u8* data;
    u8* pos;
    u8* lim;
    u8* mark;
    u32 capacity;
    bool wrapped;
    bool big_endian;
};

/**
 * Allocate new buffer data.
 *
 * capacity bytes will be allocated to store buffer data. The buffer
 * position and limit will start at zero.
 * @param buff the buffer
 * @param capacity the capacity wanted for the buffer
 * @return E_BUFF_ERR if there was not enough memory to allocate
 * @pre buff != NULL
 * @post e_buff_get_pos(buff) == 0
 * @post e_buff_get_lim(buff) == 0
 */
extern e_ret e_buff_alloc(e_buff *buff, u32 capacity);

/**
 * Wraps a pre-allocated byte array in a buffer.
 *
 * The byte array data shouldn't be accessed while the
 * buffer still references it.
 *
 * @param buff the buffer
 * @param data the pre-allocated byte array
 * @param capacity the size of the pre-allocated buffer
 * @pre buff != NULL
 * @pre data != NULL
 * @post e_buff_get_pos(buff) == 0
 * @post e_buff_get_lim(buff) == 0
 */
extern void e_buff_wrap(e_buff *buff, u8 *data, u32 capacity);

/* FIXME you should also be allowed to define the capacity and
 * the slice position should also start at 0.
 */
/**
 * Returns a view (slice) over an existing buffer of a given size.
 *
 * Both the slice and the buffer will keep a reference to the same
 * internal byte array, but each buffer will keep its own pos,
 * limit and mark.
 *
 * You should create a slice when you want to access a buffer
 * while making sure that you don't read data outside a specific
 * range.
 *
 * @param buff the buffer
 * @param slice the slice to initialize.
 * @param size the size for the slice.
 * @return OK on success
 *          E_BUFF_OVERFLOW if the size is greater than the remaining
 *          read data in the buffer.
 */
extern e_ret e_buff_slice(e_buff *buff, e_buff *slice, u32 size);

/**
 * Frees a buffer.
 *
 * Limit and pos will be set to zero and the mark will be discarded.
 *
 * If the buffer was allocated using e_buff_alloc, it internal
 * byte array will be freed.
 *
 * If the buffer is NULL, no operation is performed.
 *
 * @param buff the buffer.
 */
extern void e_buff_dealloc(e_buff *buff);

extern e_ret e_buff_read(e_buff *buff, u8* b);

extern e_ret e_buff_read_u16(e_buff *buff, u16* d);

extern e_ret e_buff_read_u32(e_buff *buff, u32* d);

extern e_ret e_buff_read_buff(e_buff *buff, u8* b, u32 size);

extern e_ret e_buff_read_asciibuff(e_buff *buff, ascii* b, u32 size);

extern e_ret e_buff_read_ascii(e_buff *buff, ascii* b, u32 max_size);

extern u32 e_buff_read_remain(e_buff *buff);

extern e_ret e_buff_write(e_buff *buff, u8 b);

extern e_ret e_buff_write_u16(e_buff *buff, u16 d);

extern e_ret e_buff_write_u32(e_buff *buff, u32 d);

extern e_ret e_buff_write_buff(e_buff *buff, u8* b, u32 size);

extern e_ret e_buff_write_asciibuff(e_buff *buff, const ascii* b, u32 size);

extern e_ret e_buff_write_ascii(e_buff *buff, const ascii* b);

extern u32 e_buff_write_remain(e_buff *buff);

extern e_ret e_buff_displace_fwd(e_buff *buff, u32 pos, u32 len);

/* FIXME: should implement a circular buffer to avoid expensive compact ops */
extern void e_buff_compact(e_buff *buff);

extern e_ret e_buff_peek(e_buff *buff, u32 pos, u8* b);

extern e_ret e_buff_skip(e_buff *buff, u32 size);

extern void e_buff_mark(e_buff *buff);

extern void e_buff_restore(e_buff *buff);

extern u32 e_buff_capacity(e_buff *buff);

extern u32 e_buff_get_lim(e_buff *buff);
extern void e_buff_set_lim(e_buff *buff, u32 lim);

extern u32 e_buff_get_pos(e_buff *buff);
extern void e_buff_set_pos(e_buff *buff, u32 pos);

extern bool e_buff_is_wrapped(e_buff *buff);

extern void e_buff_reset(e_buff *buff);

extern u32 e_buff_read_hex(e_buff *buff, ascii *hex, u32 len);

extern u32 e_buff_peek_hex(e_buff *buff, ascii *hex, u32 len);

#ifdef __cplusplus
}
#endif

#endif
