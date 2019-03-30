//
// Created by Harsha Vardhan on 2019-03-29.
//



#include "../inc/mypool.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

void *worker(int *id );

pthread_mutex_t lock;
void main(){



    tpool *pool=threadpool_init(4,10);

    int i;
    for(i=0;i <10;i++) {
        int *a = Malloc(sizeof(int));
        *a=i;
        threadpool_add_work(pool, worker,a,false);
    }

    threadpool_wait(pool);
    threadpool_free(pool);
}

void *worker(int *id ){
    pthread_mutex_lock(&lock);
    printf(" Hello from thread %d\n",*id);
    pthread_mutex_unlock(&lock);
}