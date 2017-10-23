/*=============================================================================
        Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

        This software is furnished under a license and may be used and copied
        only in accordance with the terms of such license and with the
        inclusion of the above copyright notice. This software or any other
        copies thereof may not be provided or otherwise made available to any
        other person. No title to and ownership of the software is hereby
        transferred.
==============================================================================*/
#include "e_libs_config.h"

#include "e_buff.h"

#include "e_port.h"
#include "e_mem.h"

#define _INVARIANT(x) e_assert(x != NULL);\
                     e_assert(x->lim >= x->data);\
                     e_assert(x->pos >= x->data);\
                     e_assert(x->lim >= x->pos);\
                     e_assert(x->capacity >= (u32) (x->lim - x->data));\
                     e_assert((x->mark == NULL) \
                             || ((x->mark >= x->data) \
                                 && ((u32)(x->mark - x->data) <= x->capacity)));

e_ret e_buff_alloc(e_buff *buff, u32 capacity) {
    e_assert(buff != NULL);

    buff->data = e_mem_malloc(sizeof(u8) * capacity);
    if(!(buff->data)) {
        return E_BUFF_ERR;
    }
    (void) e_mem_set(buff->data, 0, sizeof(u8) * capacity);
    buff->pos = buff->data;
    buff->lim = buff->data;
    buff->mark = NULL;
    buff->capacity = capacity;
    buff->wrapped = FALSE;
    buff->big_endian = TRUE;

    _INVARIANT(buff);
    return OK;
}

void e_buff_wrap(e_buff *buff, u8 *data, u32 capacity) {
    e_assert(buff != NULL);
    e_assert(data != NULL);

    buff->data = data;
    buff->pos = buff->data;
    buff->lim = buff->data;
    buff->mark = NULL;
    buff->capacity = capacity;
    buff->wrapped = TRUE;
    buff->big_endian = TRUE;

    _INVARIANT(buff);
}

e_ret e_buff_slice(e_buff *buff, e_buff *slice, u32 size) {
    _INVARIANT(buff);
    e_assert(slice != NULL);

    if (size > e_buff_read_remain(buff)) {
        return E_BUFF_OVERFLOW;
    }

    slice->data = buff->data;
    slice->pos = buff->pos;
    slice->lim = slice->pos + size;
    slice->mark = NULL;
    slice->capacity = buff->capacity;
    slice->wrapped = TRUE;
    slice->big_endian = buff->big_endian;

    buff->pos += size;

    _INVARIANT(buff);
    _INVARIANT(slice);
    return OK;
}

void e_buff_dealloc(e_buff *buff) {
    if (buff == NULL) {
        return;
    }

    if (!(buff->wrapped)) {
        e_mem_free(buff->data);
    }
    buff->data = NULL;
    buff->pos = NULL;
    buff->lim = NULL;
    buff->mark = NULL;
    buff->capacity = 0;
}

bool e_buff_is_wrapped(e_buff *buff) {
    _INVARIANT(buff);
    return buff->wrapped;
}

e_ret e_buff_read(e_buff *buff, u8* b) {
    _INVARIANT(buff);

    if (e_buff_read_remain(buff) < 1) {
        return E_BUFF_OVERFLOW;
    }

    *b = *(buff->pos);
    (buff->pos)++;

    _INVARIANT(buff);
    return OK;
}

e_ret e_buff_read_u16(e_buff *buff, u16* d) {
    _INVARIANT(buff);

    if (e_buff_read_remain(buff) < 2) {
        return E_BUFF_OVERFLOW;
    }

    if (buff->big_endian) {
        *d = ((buff->pos[0] & 0xFF) << 8) |
             ((buff->pos[1] & 0xFF));
    } else {
        *d = ((buff->pos[0] & 0xFF)) |
             ((buff->pos[1] & 0xFF) << 8);
    }

    buff->pos += 2;

    _INVARIANT(buff);
    return OK;
}

e_ret e_buff_read_u32(e_buff *buff, u32* d) {
    _INVARIANT(buff);

    if (e_buff_read_remain(buff) < 4) {
        return E_BUFF_OVERFLOW;
    }

    if (buff->big_endian) {
        *d = ((buff->pos[0] & 0xFF) << 24) |
             ((buff->pos[1] & 0xFF) << 16) |
             ((buff->pos[2] & 0xFF) << 8) |
             ((buff->pos[3] & 0xFF));
    } else {
        *d = ((buff->pos[0] & 0xFF)) |
             ((buff->pos[1] & 0xFF) << 8) |
             ((buff->pos[2] & 0xFF) << 16) |
             ((buff->pos[3] & 0xFF) << 24);
    }

    buff->pos += 4;

    _INVARIANT(buff);
    return OK;
}

e_ret e_buff_read_asciibuff(e_buff *buff, ascii* b, u32 size) {
    _INVARIANT(buff);

    e_assert(b != NULL);

    if (e_buff_read_remain(buff) < size) {
        return E_BUFF_OVERFLOW;
    }

    (void) e_mem_cpy(b, buff->pos, size);
    buff->pos += size;

    _INVARIANT(buff);
    return OK;
}

e_ret e_buff_read_ascii(e_buff *buff, ascii* b, u32 max_size) {
    e_ret rc = ERROR;
    u32 len;
    u32 read_remain;
    u8 *start;
    _INVARIANT(buff);

    e_assert(b != NULL);

    if (0 == max_size) {
        goto exit;
    }

    read_remain = e_buff_read_remain(buff);
    if (0 == read_remain) {
        rc = E_BUFF_OVERFLOW;
        goto exit;
    }

    start = buff->pos;

    len = max_size > read_remain ? read_remain : max_size;
    /* read until the first '\0' is found */
    while (len-- > 0) {
        *b = (ascii) (*buff->pos++);
        if ('\0' == *b) {
            break;
        }
        b++;
    }

    /* if no '\0' is found, rollback buffer and exit */
    if (*b != '\0') {
        buff->pos = start;
        goto exit;
    }

    rc = OK;

exit:
    _INVARIANT(buff);
    return rc;
}

e_ret e_buff_read_buff(e_buff *buff, u8* b, u32 size) {
    _INVARIANT(buff);

    e_assert(b != NULL);

    if (e_buff_read_remain(buff) < size) {
        return E_BUFF_OVERFLOW;
    }

    (void) e_mem_cpy(b, buff->pos, size);
    buff->pos += size;

    _INVARIANT(buff);
    return OK;
}

e_ret e_buff_peek(e_buff *buff, u32 pos, u8* b) {
    _INVARIANT(buff);

    if (e_buff_read_remain(buff) < pos) {
        return E_BUFF_OVERFLOW;
    }

    *b = *(buff->pos+pos);

    _INVARIANT(buff);
    return OK;
}

e_ret e_buff_write(e_buff *buff, u8 b) {
    _INVARIANT(buff);

    if (e_buff_write_remain(buff) < 1) {
        return E_BUFF_OVERFLOW;
    }

    *(buff->lim) = b;
    (buff->lim)++;

    _INVARIANT(buff);
    return OK;
}

e_ret e_buff_write_u16(e_buff *buff, u16 d) {
    _INVARIANT(buff);

    if (e_buff_write_remain(buff) < 2) {
        return E_BUFF_OVERFLOW;
    }
    if (buff->big_endian) {
        buff->lim[0] = (u8) ((d >> 8) & 0xFF);
        buff->lim[1] = (u8) ((d) & 0xFF);
    } else {
        buff->lim[0] = (u8) ((d) & 0xFF);
        buff->lim[1] = (u8) ((d >> 8) & 0xFF);
    }

    buff->lim += 2;

    _INVARIANT(buff);
    return OK;
}

e_ret e_buff_write_u32(e_buff *buff, u32 d) {
    _INVARIANT(buff);

    if (e_buff_write_remain(buff) < 4) {
        return E_BUFF_OVERFLOW;
    }

    if (buff->big_endian) {
        buff->lim[0] = (d >> 24) & 0xFF;
        buff->lim[1] = (d >> 16) & 0xFF;
        buff->lim[2] = (d >> 8) & 0xFF;
        buff->lim[3] = (d) & 0xFF;
    } else {
        buff->lim[0] = (d) & 0xFF;
        buff->lim[1] = (d >> 8) & 0xFF;
        buff->lim[2] = (d >> 16) & 0xFF;
        buff->lim[3] = (d >> 24) & 0xFF;
    }

    buff->lim += 4;

    _INVARIANT(buff);
    return OK;
}

e_ret e_buff_write_buff(e_buff *buff, u8* b, u32 size) {
    _INVARIANT(buff);
    e_assert(b != NULL);

    if (e_buff_write_remain(buff) < size) {
        return E_BUFF_OVERFLOW;
    }

    (void) e_mem_cpy(buff->lim, b, size);
    buff->lim += size;

    _INVARIANT(buff);
    return OK;
}

e_ret e_buff_write_asciibuff(e_buff *buff, const ascii* b, u32 size) {
    _INVARIANT(buff);
    e_assert(b != NULL);

    if (e_buff_write_remain(buff) < size) {
        return E_BUFF_OVERFLOW;
    }

    (void) e_mem_cpy(buff->lim, b, size);
    buff->lim += size;

    _INVARIANT(buff);
    return OK;
}

e_ret e_buff_write_ascii(e_buff *buff, const ascii* b) {
    size_t len;
    u32 size;
    _INVARIANT(buff);
    e_assert(b != NULL);

    len = e_strlen(b);
    if (len >= 0xFFFFFFFFu) {
        return E_BUFF_OVERFLOW;
    }

    size = (len & 0xFFFFFFFFu) + 1;
    if (e_buff_write_remain(buff) < size) {
        return E_BUFF_OVERFLOW;
    }

    (void) e_mem_cpy(buff->lim, b, size - 1);
    buff->lim += size - 1;
    *(buff->lim) = (u8) '\0';
    buff->lim++;

    _INVARIANT(buff);
    return OK;

}

e_ret e_buff_displace_fwd(e_buff *buff, u32 pos, u32 len) {
    u32 lim;
    _INVARIANT(buff);

    e_assert(len > 0);

    if (e_buff_get_pos(buff) > pos) {
        return E_BUFF_ERR;
    }
    if (e_buff_get_lim(buff) < pos) {
        return E_BUFF_ERR;
    }
    if (e_buff_capacity(buff) < pos + len) {
        return E_BUFF_OVERFLOW;
    }

    lim = e_buff_get_lim(buff);
    /* move all bytes from pos to lim, len positions */
    (void) e_mem_move(
            buff->data + pos + len,
            buff->data + pos,
            lim - pos);
    e_buff_set_lim(buff, lim + len);

    _INVARIANT(buff);

    return OK;
}

void e_buff_compact(e_buff *buff) {
    u32 r;
    _INVARIANT(buff);

    r = e_buff_read_remain(buff);
    (void) e_mem_move(buff->data, buff->pos, r);

    buff->pos = buff->data;
    buff->lim = buff->data + r;
    buff->mark = NULL;

    _INVARIANT(buff);
}

void e_buff_mark(e_buff *buff) {
    _INVARIANT(buff);

    buff->mark = buff->pos;

    _INVARIANT(buff);
}

void e_buff_restore(e_buff *buff) {
    _INVARIANT(buff);
    e_assert( buff->mark != NULL );

    buff->pos = buff->mark;
    buff->mark = NULL;

    _INVARIANT(buff);
}

e_ret e_buff_skip(e_buff *buff, u32 size) {
    _INVARIANT(buff);

    if (e_buff_read_remain(buff) < size) {
        return E_BUFF_OVERFLOW;
    }

    buff->pos += size;

    _INVARIANT(buff);
    return OK;
}

u32 e_buff_write_remain(e_buff *buff) {
    _INVARIANT(buff);

    return buff->capacity - (u32)((buff->lim - buff->data));
}

u32 e_buff_read_remain(e_buff *buff) {
    _INVARIANT(buff);

    return (u32) (buff->lim - buff->pos);
}

u32 e_buff_capacity(e_buff *buff) {
    _INVARIANT(buff);

    return buff->capacity;
}

void e_buff_reset(e_buff *buff) {
    _INVARIANT(buff);

    buff->pos = buff->data;
    buff->lim = buff->data;

    _INVARIANT(buff);
}

void e_buff_set_pos(e_buff *buff, u32 pos) {
    _INVARIANT(buff);
    e_assert(pos <= buff->capacity);

    buff->pos = buff->data + pos;

    _INVARIANT(buff);
}

void e_buff_set_lim(e_buff *buff, u32 lim) {
    _INVARIANT(buff);
    e_assert(lim <= buff->capacity);

    buff->lim = buff->data + lim;

    _INVARIANT(buff);
}

u32 e_buff_get_lim(e_buff *buff) {
    _INVARIANT(buff);

    return (u32) (buff->lim - buff->data);
}

u32 e_buff_get_pos(e_buff *buff) {
    _INVARIANT(buff);

    return (u32) (buff->pos - buff->data);
}

u32 e_buff_read_hex(e_buff *buff, ascii *hex, u32 len) {
	u32 r;
	r = e_buff_peek_hex(buff, hex, len);
	buff->pos += r/2;
	_INVARIANT(buff);
	return r;
}

u32 e_buff_peek_hex(e_buff *buff, ascii *hex, u32 len) {
	static char __hexval[16] = {
			'0', '1', '2', '3', '4', '5', '6', '7',
	        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
	u32 i;
	u32 remain;
	u32 oct_len;

	_INVARIANT(buff);
	e_assert(hex != NULL);

	if (len == 0) {
		return 0;
	}

	oct_len = ((len % 2) == 0) ? ((len / 2) - 1) : (len / 2);

	remain = e_buff_read_remain(buff);
	if (oct_len > remain) {
		oct_len = remain;
	}

	for (i = 0; i < oct_len; i++) {
		hex[i*2] = __hexval[(int)((buff->pos[i] >> 4) & 0xF)];
		hex[(i*2) + 1] = __hexval[(int)(buff->pos[i]) & 0xF];
	}
	hex[oct_len*2] = '\0';

	return oct_len*2;
}
