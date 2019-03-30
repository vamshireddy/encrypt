//
// Created by Harsha Vardhan on 2019-03-30.
//

#ifndef ENCRYPT_WRAPPERS_H
#define ENCRYPT_WRAPPERS_H

#endif //ENCRYPT_WRAPPERS_H

#include <stdlib.h>
#include <sys/mman.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <memory.h>

void Sem_init(sem_t *sem, int pshared, unsigned int value);

/* Dynamic storage allocation wrappers */
void *Malloc(size_t size);
void Free(void *ptr);


/* Pthreads thread control wrappers */
void Pthread_create(pthread_t *tidp, pthread_attr_t *attrp,
                    void * (*routine)(void *), void *argp);
void Pthread_join(pthread_t tid, void **thread_return);
void Pthread_detach(pthread_t tid);

/* POSIX semaphore wrappers */
void Sem_init(sem_t *sem, int pshared, unsigned int value);

/* Standard I/O wrappers */
void Fclose(FILE *fp);
FILE *Fdopen(int fd, const char *type);
char *Fgets(char *ptr, int n, FILE *stream);
FILE *Fopen(const char *filename, const char *mode);
void Fputs(const char *ptr, FILE *stream);
size_t Fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
void Fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);


/* Unix I/O wrappers */
int Open(const char *pathname, int flags, mode_t mode);
ssize_t Read(int fd, void *buf, size_t count);
ssize_t Write(int fd, const void *buf, size_t count);
off_t Lseek(int fildes, off_t offset, int whence);
void Close(int fd);


/* Our own error-handling functions */
void unix_error(char *msg);
void posix_error(int code, char *msg);
