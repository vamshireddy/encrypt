//
// Created by Harsha Vardhan on 2019-03-30.
//

#include "../include/wrappers.h"



/*
 * unix style error . prints the error to screen
 */
void unix_error(char *msg)
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(0);
}


void posix_error(int code, char *msg) /* Posix-style error */
{
    fprintf(stderr, "%s: %s\n", msg, strerror(code));
    exit(0);
}

void Sem_init(sem_t *sem, int pshared, unsigned int value){
    if (sem_init(sem, pshared, value) < 0)
        unix_error("Sem_init error");
}



void Pthread_create(pthread_t *tidp, pthread_attr_t *attrp,
                    void * (*routine)(void *), void *argp)
{
    int rc;

    if ((rc = pthread_create(tidp, attrp, routine, argp)) != 0)
        posix_error(rc, "Pthread_create error");
}

void Pthread_join(pthread_t tid, void **thread_return) {
    int rc;

    if ((rc = pthread_join(tid, thread_return)) != 0)
        posix_error(rc, "Pthread_join error");
}


void *Malloc(size_t size)
{
    void *p;

    if ((p  = malloc(size)) == NULL)
        unix_error("Malloc error");
    return p;
}

void Free(void *ptr)
{
    free(ptr);
}


FILE *Fopen(const char *filename, const char *mode)
{
    FILE *fp;

    if ((fp = fopen(filename, mode)) == NULL)
        unix_error("Fopen error");

    return fp;
}


size_t Fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t n;

    if (((n = fread(ptr, size, nmemb, stream)) < nmemb) && ferror(stream))
        unix_error("Fread error");
    return n;
}

void Fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    if (fwrite(ptr, size, nmemb, stream) < nmemb)
        unix_error("Fwrite error");
}


