//
// Created by Harsha Vardhan on 2019-03-29.
//

#ifndef ENCRYPT_MYPOOL_H
#define ENCRYPT_MYPOOL_H

#endif //ENCRYPT_MYPOOL_H


#define  QUE_SIZE 20

#include "sharedbuffer.h"
#include <stdbool.h>


struct tpool {

    int active_threads;
    shared_buffer *workque;
    int que_size;
    int numThreads;
    int workingTrheads;
    int workinQue;
    pthread_mutex_t lock;
    pthread_t **threads;
};


struct work {
    void   (*workfunction)(void* arg);
    void*  argument;
    bool end;
};
typedef struct tpool tpool;
typedef struct work work;


tpool * threadpool_init(int numThreads,int queSize);
void threadpool_add_work(tpool *pool,void (*function_p)(void*), void* arg_p,bool end);
void threadpool_wait(tpool *pool);
void threadpool_free(tpool *pool);


