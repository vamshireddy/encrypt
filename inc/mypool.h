//
// Created by Harsha Vardhan on 2019-03-29.
//

#ifndef ENCRYPT_MYPOOL_H
#define ENCRYPT_MYPOOL_H

#endif //ENCRYPT_MYPOOL_H


#define  QUE_SIZE 20

#include "sharedbuffer.h"
#include <stdbool.h>


/*
 *  Struct to store the thread pool parametrs
 *
 */
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

/*
 *  To package the work into a generic struct so that we can use same
 *  shared buffer for everyting
 */

struct work {
    void   (*workfunction)(void* arg);
    void*  argument;
    bool end;
};

typedef struct tpool tpool;
typedef struct work work;

/*
 * @brief initialises the thread pool with given number of threads and the size of the internal que for jobs
 *
 */
tpool * threadpool_init(int num_threads,int que_size);

/*
 *  @brief adds the work to the internal job que so that any free thread can pick it up .
 *  @param pool  thread pool
 *  @param function-p function pointer that is getting executd
 *  @param end implicit signal to picking up thread to die / kill
 *
 */
void threadpool_add_work(tpool *pool,void (*function_p)(void*), void* arg_p,bool end);

/*
 * @brief waits for all the threads to finish their work
 *
 */
void threadpool_wait(tpool *pool);

/*
 * waits for all the threads to finish their work and joins all threads and frees up resources
 */
void threadpool_free(tpool *pool);


