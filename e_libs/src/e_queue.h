/*=============================================================================
        Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

        This software is furnished under a license and may be used and copied
        only in accordance with the terms of such license and with the
        inclusion of the above copyright notice. This software or any other
        copies thereof may not be provided or otherwise made available to any
        other person. No title to and ownership of the software is hereby
        transferred.
==============================================================================*/
#ifndef _e_queue_h_
#define _e_queue_h_

#include "e_port.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _e_queue *e_queue;

/**
 * Creates a new queue.
 *
 * @return OK if the queue was created successfully, 
 * ERROR if no memory was available for the queue.
 * @pre q != NULL
 * @pre *q == NULL
 * @post e_queue_size(q) == 0
 */
extern e_ret e_queue_create(e_queue *q);

/**
 * Frees the e_queue.
 * @pre e_queue_size(q) == 0
 * @post *q == NULL
 */
extern void e_queue_free(e_queue *q);

/**
 * Puts data at the tail of the queue
 *
 * @param q the queue
 * @param data the data
 * @pre data != NULL
 * @pre q != NULL
 */
extern void e_queue_put_last(e_queue q, void* data);

/**
 * Puts data at the head of the queue
 *
 * @param q the queue
 * @param data the data
 * @pre data != NULL
 * @pre q != NULL
 */
extern void e_queue_put_first(e_queue q, void* data);

/**
 * Returns the head of the queue, or NULL if none.
 *
 * @param q the queue
 * @return the first element
 * @pre q != NULL
 */
extern void *e_queue_head(e_queue q);

/**
 * Returns the tail of the queue, or NULL if none.
 *
 * @param q the queue
 * @return the last element
 * @pre q != NULL
 */
extern void *e_queue_tail(e_queue q);

/**
 * Removes and returns the head of the queue, or NULL if none.
 *
 * @param q the queue
 * @return the element or NULL if the queue is empty.
 * @pre q != NULL
 */
extern void *e_queue_pop_first(e_queue q);

/**
 * Removes and returns the last element of the queue, or NULL if none.
 *
 * @param q the queue
 * @return the element or NULL if the queue is empty.
 * @pre q != NULL
 */
extern void *e_queue_pop_last(e_queue q);

/*
 * Returns the size of the queue.
 *
 * @param q the queue
 * @return the number of elements in the queue
 * @pre q != NULL
 */
extern u16 e_queue_size(e_queue q);

#ifdef __cplusplus
}
#endif

#endif
