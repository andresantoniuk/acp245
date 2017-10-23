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

#include "e_queue.h"

#include "e_port.h"
#include "e_log.h"
#include "e_mem.h"

typedef struct _node {
    void* data;
    struct _node *next;
    struct _node *prev;
} _node;

struct _e_queue {
    _node* head;
    _node* tail;
    u16 size;
};

static void _node_free(_node* node)
{
    e_mem_free(node);
}

e_ret e_queue_create(e_queue *q)
{
    *q = e_mem_malloc(sizeof(struct _e_queue));
    if (!*q) {
        E_LOG_NOMEM;
        return ERROR;
    }
    (void) e_mem_set(*q, 0, sizeof(struct _e_queue));
    return OK;
}

u16 e_queue_size(e_queue q)
{
    return q->size;
}

void* e_queue_head(e_queue q)
{
    if (q->head) {
        return q->head->data;
    } else {
        return NULL;
    }
}

void* e_queue_tail(e_queue q)
{
    if (q->tail) {
        return q->tail->data;
    } else {
        return NULL;
    }
}

void e_queue_free(e_queue *q)
{
    if (q == NULL || *q == NULL) {
        return;
    }

    e_assert((*q)->head == NULL);
    e_assert((*q)->tail == NULL);
    e_assert((*q)->size == 0);

    e_mem_free(*q);

    *q = NULL;
}

void* e_queue_pop_first(e_queue q)
{
    void* data;
    _node* n;

    if (q->head == NULL) {
        return NULL;
    }

    n = q->head;
    data = n->data;
    q->head = n->next;
    if (q->head) {
        q->head->prev = NULL;
    } else {
        q->tail = NULL;
    }
    _node_free(n); 

    q->size--;

    return data;
}

void* e_queue_pop_last(e_queue q)
{
    void* data;
    _node* n;

    if (q->tail == NULL) {
        return NULL;
    }
    
    n = q->tail;
    data = n->data;
    q->tail = n->prev;
    if (q->tail) {
        q->tail->next = NULL;
    } else {
        q->head = NULL;
    }
    _node_free(n); 

    q->size--;

    return data;
}

void e_queue_put_last(e_queue q, void * data)
{
    _node* n;

    e_assert(q != NULL);
    e_assert(data != NULL);

    n = e_mem_malloc(sizeof(_node));
    if(!n) {
        E_LOG_NOMEM;
        return;
    }

    n->data = data;
    n->next = NULL;
    n->prev = q->tail;

    if (q->head == NULL) {
        q->head = n;
    } else {        
        q->tail->next = n;
    }
    q->tail = n;
    q->size++;
}

void e_queue_put_first(e_queue q, void * data)
{
    _node *n;

    e_assert(q != NULL);
    e_assert(data != NULL);

    n = e_mem_malloc(sizeof(_node));
    if(!n) {
        E_LOG_NOMEM;
        return;
    }

    n->data = data;
    n->next = q->head;
    n->prev = NULL;

    if (q->head == NULL) {
        q->tail = n;
    } else {
        q->head->prev = n;
    }
    q->head = n;
    q->size++;
}
