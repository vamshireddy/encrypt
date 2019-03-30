//
// Created by Harsha Vardhan on 2019-03-29.
//

#ifndef ENCRYPT_SHAREDBUFFER_H
#define ENCRYPT_SHAREDBUFFER_H

#endif //ENCRYPT_SHAREDBUFFER_H

#include <semaphore.h>

/*
 *
 *  shard buffer that follows fifo discipline
 */
typedef struct {
    void **buf;     /* Buffer array                       */
    int n;        /* Maximum number of slots            */
    int front;    /* buf[front+1 (mod n)] is first item */
    int rear;     /* buf[rear]   is last item           */
    sem_t mutex;  /* Protects accesses to buf           */
    sem_t slots;  /* Counts available slots             */
    sem_t items;  /* Counts available items             */
} shared_buffer;


/*
 * initialises the buffer with given capacity
 */
void sharedbuffer_init(shared_buffer *sp, int n);
/*
 * frees up resources
 */
void sharedbuffer_free(shared_buffer *sp);
/*
 * adds the item to the buffer if its not free the call gets blocked
 */
void sharebuffer_insert(shared_buffer *sp, void *item);
/*
 * remove the item from the buffer if there is nothign to remove the caller gets blocked.
 */
void *sharedbuffer_remove(shared_buffer *sp);
