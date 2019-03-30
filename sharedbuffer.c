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
#include "sharedbuffer.h"
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
    // Buffer holds max of n items
    sp->n = n;
    //Empty buffer iff front == rear
    sp->front = sp->rear = 0;
    // Binary semaphore for locking
    sem_init(&sp->mutex, 0, 1);
    // Initially, buf has n empty slots
    sem_init(&sp->slots, 0, n);
    // Initially, buf has zero data items
    sem_init(&sp->items, 0, 0);
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
    // Wait for available slot
    sem_wait(&sp->slots);
    // Lock the buffer
    sem_wait(&sp->mutex);
    // Increment index (mod n)
    if (++sp->rear >= sp->n)
        sp->rear = 0;
    // Insert the item
    sp->buf[sp->rear] = item;
    // Unlock the buffer
    sem_post(&sp->mutex);
    // Announce available item
    sem_post(&sp->items);
}


/*
 * Remove and return the first item from buffer sp
 */

void* sharedbuffer_remove(shared_buffer *sp)
{
    void *item;
    // Wait for available item
    sem_wait(&sp->items);
    //  Lock the buffer
    sem_wait(&sp->mutex);
    // Increment index (mod n)
    if (++sp->front >= sp->n)
        sp->front = 0;
    // Remove the item
    item = sp->buf[sp->front];
    //  Unlock the buffer
    sem_post(&sp->mutex);
    // Announce available slot
    sem_post(&sp->slots);
    return item;
}
