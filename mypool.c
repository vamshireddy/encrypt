// void* arg_p
// Created by Harsha Vardhan on 2019-03-29.
//

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




static void worker_thread();
static void thread_init(tpool *pool,int id);




tpool * threadpool_init(int numThreads,int queSize){


    if (numThreads <0) numThreads=0;

    tpool *pool = malloc(sizeof(tpool));
    pool->numThreads=numThreads;
    pool->active_threads=0;
    pool->active_threads=0;
    pool->que_size=queSize;

    pool->threads = malloc(sizeof(pthread_t *)  * numThreads);
    pool->workque = malloc(sizeof(shared_buffer));

    sharedbuffer_init(pool->workque,pool->que_size);

    //intialise the threads
    int i;
    for (i=0;i<numThreads;i++){
        thread_init(pool,i);
    }

    //wait for all the threads to come online
    while(pool->active_threads != numThreads){}
    return pool;

}

void threadpool_add_work(tpool *pool,void (*function_p)(void*), void* arg_p,bool end){

        work *worktobepushed = malloc(sizeof(work));
        if(!end) {
            worktobepushed->workfunction= function_p;
            worktobepushed->argument=arg_p;
            worktobepushed->end=end;
        }else{
            worktobepushed->end=true;
        }
        sharebuffer_insert(pool->workque,worktobepushed);
        pthread_mutex_lock(&pool->lock);
        pool->workinQue++;
        pthread_mutex_unlock(&pool->lock);
}

static void thread_init(tpool *pool,int id){
    pool->threads[id] = malloc(sizeof(pthread_t));
    pthread_create(pool->threads[id], NULL, (void *(*)(void *)) worker_thread, pool);
}


void threadpool_wait(tpool *pool){

    while(pool->workingTrheads != 0 || pool->workinQue !=0){}
}


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