//
// Created by Harsha Vardhan on 2019-03-29.
//

/***********************************************************************************************************************
 *
 * Simple threadpool module that is capable of creating a pool of threads and provide minimal fucntionalities.
 * This module can :
 *          1. Create a pool of threads
 *          2. Add work to the inbuilt shared buffer , so that worker threads can pick it up
 *          3. Wait for threads to complete their work
 *          4. Free up the resources and join all the threds before exit
 * The heart of this lies the shared buffer with a FIFI discipline. A random thread will pick up the work
 * as it is inserted into the work buffer and finishes it .
 *
 * A simple yet novel technique is used to signal the threads to exit the forever loop , a dummy work struct with
 * a flag is inserted into the buffer which signals the thread that picked up the buffer to exit the loop .
 * A simple way to signal or kill the threads.
 *
 */

#include "mypool.h"
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <semaphore.h>
#include <stdbool.h>


static void worker_thread(tpool *);
static void thread_init(tpool *pool,int id);


/*
 *  @brief initialise the thread pool with given number of threads and the specified size of the
 *  internal work buffer
 *
 */
tpool * threadpool_init(int numThreads,int queSize){

    if (numThreads <0) numThreads=0;

    tpool *pool = malloc(sizeof(tpool));
    pool->numThreads=numThreads;
    pool->active_threads=0;
    pool->active_threads=0;
    pool->que_size=queSize;

    // array of worker threads
    pool->threads = malloc(sizeof(pthread_t *)  * numThreads);
    pool->workque = malloc(sizeof(shared_buffer));

    // shard buffer to keep track of the work that is being inserted
    sharedbuffer_init(pool->workque,pool->que_size);

    // intialise the threads
    int i;
    for (i=0;i<numThreads;i++){
        thread_init(pool,i);
    }

    //wait for all the threads to come online
    while(pool->active_threads != numThreads){}
    return pool;

}

/*
 *  @brief adds the work to the shared buffer for the workes to be picked up
 */
void threadpool_add_work(tpool *pool,void (*function_p)(void*), void* arg_p,bool end){

        work *work_to_be_done = malloc(sizeof(work));
        if(!end) {
            work_to_be_done->workfunction= function_p;
            work_to_be_done->argument=arg_p;
            work_to_be_done->end=end;
        }else{
            // kill signal to the worker to exit
            work_to_be_done->end=true;
        }
        sharebuffer_insert(pool->workque,work_to_be_done);
        pthread_mutex_lock(&pool->lock);
        pool->workinQue++;
        pthread_mutex_unlock(&pool->lock);
}

static void thread_init(tpool *pool,int id){
    pool->threads[id] = malloc(sizeof(pthread_t));
    pthread_create(pool->threads[id], NULL, (void *(*)(void *)) worker_thread, pool);
}


/*
 *  @brief blocks until all the threds finish their picked up work
 */
void threadpool_wait(tpool *pool){

    while(pool->workingTrheads != 0 || pool->workinQue !=0){}
}


/*
 * Makes sure all the worker threds exit , free up the resources and exit .
 *
 */
void threadpool_free(tpool *pool){
    int i=0;
    int numThreads = pool->numThreads;
    for(i=0;i<numThreads;i++){
        work *temp = malloc(sizeof(work));
        temp->end = true;
        sharebuffer_insert(pool->workque,temp);
    }
    for( i=0;i< numThreads;i++)
    {
        pthread_join(*pool->threads[i],NULL);
    }
    free(pool->workque);
    free(pool->threads);
    free(pool);
    printf("Good bye ! \n");
}

/*
 * @brief worker thread that actually picks up the work and does it .
 * It is in a forever loop until signalled to exit.
 * It blocks on the shared buffer when there is no work
 *
 */
static void worker_thread(tpool *pool){

    pthread_mutex_lock(&pool->lock);
    pool->active_threads++;
    pthread_mutex_unlock(&pool->lock);


    while(1){

        work *workwaiting = sharedbuffer_remove(pool->workque);
        // do something
         pthread_mutex_lock(&pool->lock);
         pool->workingTrheads++;
         pool->workinQue--;
         pthread_mutex_unlock(&pool->lock);

        void (*function_pointer)(void*);
        void*  argument_pointer;

        if(workwaiting->end){
            break;
        }

        function_pointer = workwaiting->workfunction;
        argument_pointer=workwaiting->argument;
        function_pointer(argument_pointer);

        free(workwaiting);
        //finished the work
        pthread_mutex_lock(&pool->lock);
        pool->workingTrheads--;
        pthread_mutex_unlock(&pool->lock);

    }
    pthread_mutex_lock(&pool->lock);
    pool->workingTrheads--;
    pool->active_threads--;
    pthread_mutex_unlock(&pool->lock);
}