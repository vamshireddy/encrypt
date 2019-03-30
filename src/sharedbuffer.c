//
// Created by Harsha Vardhan on 2019-03-29.
//

/***********************************************************************************************************************
 *                                              SHARD BUFFER
 *  This module provides a circular buffer that can be shared between a set of threads primarily in a producer
 *  consumer setting.
 *  The consumers gets blocked if the buffer is empty and the producers block until atleast a single slot is availiable
 *  for insertion .
 *
 *
 *
 */
#include "../include/sharedbuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>


/*
 * Create an empty, bounded, shared FIFO buffer with n slots
 *
 */

void sharedbuffer_init(shared_buffer *sp, int n)
{
    sp->buf = calloc(n, sizeof(void *));
    sp->n = n;                       /* Buffer holds max of n items        */
    sp->front = sp->rear = 0;        /* Empty buffer iff front == rear     */
    sem_init(&sp->mutex, 0, 1);      /* Binary semaphore for locking       */
    sem_init(&sp->slots, 0, n);      /* Initially, buf has n empty slots   */
    sem_init(&sp->items, 0, 0);      /* Initially, buf has zero data items */
}


/*
 * Clean up buffer sp
 *
 */

void sharedbuffer_free(shared_buffer *sp)
{
    free(sp->buf);
}


/*
 * Insert item onto the rear of shared buffer sp
 *
 */

void sharebuffer_insert(shared_buffer *sp, void *item)
{
    sem_wait(&sp->slots);               /* Wait for available slot */
    sem_wait(&sp->mutex);               /* Lock the buffer         */
    if (++sp->rear >= sp->n)            /* Increment index (mod n) */
        sp->rear = 0;
    sp->buf[sp->rear] = item;           /* Insert the item         */
    sem_post(&sp->mutex);               /* Unlock the buffer       */
    sem_post(&sp->items);               /* Announce available item */
}


/*
 * Remove and return the first item from buffer sp
 */

void* sharedbuffer_remove(shared_buffer *sp)
{
    void *item;
    sem_wait(&sp->items);               /* Wait for available item */
    sem_wait(&sp->mutex);               /* Lock the buffer         */
    if (++sp->front >= sp->n)           /* Increment index (mod n) */
        sp->front = 0;
    item = sp->buf[sp->front];          /* Remove the item         */
    sem_post(&sp->mutex);               /* Unlock the buffer       */
    sem_post(&sp->slots);               /* Announce available slot */
    return item;
}
