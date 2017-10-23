/*=============================================================================
        Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

        This software is furnished under a license and may be used and copied
        only in accordance with the terms of such license and with the
        inclusion of the above copyright notice. This software or any other
        copies thereof may not be provided or otherwise made available to any
        other person. No title to and ownership of the software is hereby
        transferred.
==============================================================================*/
#include "acp245_config.h"

#include "acp_el.h"

#include "acp_err.h"
#include "acp_ie.h"
#include "e_buff.h"
#include "e_log.h"
#include "e_mem.h"

static E_INLINE void acp_el_free_tcu_data_error_item(acp_el_tcu_data_error_item *el) {
    e_assert( el != NULL );

    acp_el_free_error(&el->error);

    e_mem_free(el->data);
    el->data = NULL;
    el->data_len = 0;
    el->type = 0;
}

static e_ret acp_el_read_tcu_data_error_item(e_buff *buff, acp_el_tcu_data_error_item *el) {
    e_ret rc;

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_read_tcu_data_error_item enter.");

    if(e_buff_read_u16(buff, &el->type)) {
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }
    if(e_buff_read(buff, &el->data_len)) {
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }
    el->data = e_mem_malloc(sizeof(u8) * el->data_len);
    if (!el->data) {
        el->data_len = 0;
        rc = ACP_MSG_ERR_NO_MEM;
        goto exit;
    }
    if (e_buff_read_buff(buff, el->data, el->data_len)) {
        e_mem_free(el->data);
        el->data = NULL;
        el->data_len = 0;
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }

    if ((rc = acp_el_read_error(buff, &el->error))) {
        e_mem_free(el->data);
        el->data = NULL;
        el->data_len = 0;
        goto exit;
    }

    rc = OK;

exit:
    E_TRACE("acp_el_read_tcu_data_error_item exit.");
    return rc;
}

static e_ret acp_el_write_tcu_data_error_item(e_buff *buff, acp_el_tcu_data_error_item *el){
    e_ret rc;

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_write_tcu_data_error_item enter.");

    if (e_buff_write_u16(buff, el->type)) {
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }
    if (e_buff_write(buff, el->data_len)) {
        rc = ACP_MSG_ERR_INCOMPLETE;
        goto exit;
    }

    if (el->data != NULL) {
        if (e_buff_write_buff(buff, el->data, el->data_len)) {
            rc = ACP_MSG_ERR_INCOMPLETE;
            goto exit;
        }
    }

    if ((rc = acp_el_write_error(buff, &el->error))) {
        goto exit;
    }

    rc = OK;

exit:
    E_TRACE("acp_el_write_tcu_data_error_item exit.");
	return rc;
}

E_INLINE void acp_el_free_tcu_data_error(acp_el_tcu_data_error *el) {
    e_assert( el != NULL );
    e_assert( el->items_cnt == 0 || el->items != NULL);

    while(el->items_cnt) {
        --el->items_cnt;
        acp_el_free_tcu_data_error_item(&el->items[el->items_cnt]);
    }

    e_mem_free(el->items);
    el->items = NULL;
}

e_ret acp_el_read_tcu_data_error(e_buff *buff, acp_el_tcu_data_error *el){
    e_ret rc;
    acp_ie ie;
    /* don't accept more than 64 data items at the same time */
    acp_el_tcu_data_error_item items[64];
    u8 cnt = 0;

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_read_tcu_data_error enter.");

    rc = acp_ie_read_exp(buff, &ie, ACP_IE_BINARY);
    if (rc || !IE_EXIST(ie)) {
        goto exit;
    }

    cnt = 0;
    while(IE_REMAIN(ie)) {
        if (cnt == 65) {
            rc = ACP_MSG_ERR_UNSUPPORTED;
            goto exit;
        }
        if((rc = acp_el_read_tcu_data_error_item(&ie.data, &items[cnt]))) {
            goto exit;
        }
        ++cnt;
    }

    /* Copy stack alloc'd data to heap alloc'd */
    el->items = e_mem_malloc(sizeof(acp_el_tcu_data_error_item) * cnt);
    if (!el->items) {
        rc = ACP_MSG_ERR_NO_MEM;
        goto exit;
    }
    (void) e_mem_cpy(el->items, items, sizeof(acp_el_tcu_data_error_item) * cnt);
    el->items_cnt = cnt;

    rc = OK;

exit:
    if (rc) {
        while(cnt) {
            --cnt;
            acp_el_free_tcu_data_error_item(&items[cnt]);
        }
    }
    E_TRACE("acp_el_read_tcu_data_error exit.");
	return rc;
}

e_ret acp_el_write_tcu_data_error(e_buff *buff, acp_el_tcu_data_error *el){
    e_ret rc;
    u32 sz;
    int i;

    e_assert( buff != NULL );
    e_assert( el != NULL );

    E_TRACE("acp_el_write_tcu_data_error enter.");

    sz = 0;
    for (i = 0; i < el->items_cnt; i++) {
        sz += 3 /* type + len */ +
            el->items[i].data_len +
            acp_el_size_error(&el->items[i].error);
    }

    if ((rc = acp_ie_write_hdr(buff, ACP_IE_BINARY, sz))) {
        goto exit;
    }

    for (i = 0; i < el->items_cnt; i++) {
        if ((rc = acp_el_write_tcu_data_error_item(buff, &el->items[i]))) {
            goto exit;
        }
    }

    rc = OK;

exit:
    E_TRACE("acp_el_write_tcu_data_error exit.");
	return rc;
}
