#include <stdio.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <pthread.h>
#include "threadpool.h"

struct tip {
    char *output;
    char *input;
    char *key;

    long chunksize;
    long inputsize;
    int thread_id;

};

typedef struct tip thread_input;

pthread_cond_t cv;
pthread_mutex_t m;
int count;


count = 0;

bool fxor(FILE *in, FILE *out, FILE *key);
void shift_1bits_left(uint8_t* array, long size);
void *getXorOutput(thread_input *threadinput);

static const char *HELP_MESSAGE = " The usage encrypt -k keyfile.bin -n 1 < plain.bin > cypher.bin";

int main(int argc, char **argv) {

    int c ;
    char *keyFile;
    long  numT;
    char *temp;
    bool verbose=false;

    while ((c = getopt(argc, argv, "k:N:d")) != EOF) {
        switch (c) {
            case 'N':           // Take number of threads as the input
                numT = strtol(optarg,&temp,10);
                break;
            case 'k':           // key file
                keyFile = malloc(sizeof(char)*strlen(optarg));
                strncpy(keyFile,optarg,strlen(optarg));
                break;
            case 'h':           // print the help message
                printf("%s",HELP_MESSAGE);
                break;
            case 'v' :
                verbose =true;
                break;
            default:
                printf("Invalid argument \n ");
                printf("%s",HELP_MESSAGE);
        }
    }

    if(verbose){
        printf("verbose output is on\n");
        printf("Key file name is %s\n",keyFile);
        printf("Number of threads are %ld\n",numT);
    }


  /*
   *
   * Binary file generator
   *
  */

    /*
    char inputdata[] = {0xF0,0xF0};
    int inputfile = open("/Users/harsha/CLionProjects/encrypt/keyfile",O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    for(int i =0;i <2;i++){
        write(inputfile,inputdata,2);
    }
    */

    /*
    char inputdata[] = {0x1,0x2,0x3,0x4,0x11,0x12,0x13,0x14,0x1,0x2,0x3,0x4,0x11,0x12,0x13,0x14,0x1,0x2,0x3,0x4,0x11,0x12,0x13,0x14};
    int inputfile = open("/Users/harsha/CLionProjects/encrypt/input",O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
     write(inputfile,inputdata,24);
    */




  /*
   * Make the dummy input ready for testing
   */


   int NUM_THREADS = 4;
   FILE *inputfile = fopen("/Users/harsha/CLionProjects/encrypt/input","rb");

   FILE *keyfile = fopen("/Users/harsha/CLionProjects/encrypt/keyfile","rb");
   fseek(keyfile,0L,SEEK_END);
   long chunk_size = ftell(keyfile);
   rewind(keyfile);


   unsigned  char keybuffer[chunk_size];
   for(int i=0;i<chunk_size;i++){
       keybuffer[i] = fgetc(keyfile);
   }
   // End of dummy input



   char outputbuffer[32];                // final output buffer that all threads output will be written to
   void *status;
   printf("entering main loop\n");
   int pos=0;                           //global buffer position

   //create a thread pool with NUM_THREADS
   threadpool_t *pool = threadpool_create(NUM_THREADS,4*NUM_THREADS,0);


    int outwrite = open("/Users/harsha/CLionProjects/encrypt/output",O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

   while(!feof(stdin)) {

       //pthread_t *threads = malloc(sizeof(pthread_t)* NUM_THREADS);
       thread_input *inputs = malloc(sizeof(thread_input) * NUM_THREADS);
        int spawned_threads=0;

        while(!feof(stdin) && spawned_threads < NUM_THREADS){
            printf("entering second main \n");
            thread_input *tempStore = inputs+spawned_threads;
            tempStore->input= malloc(sizeof(char) * chunk_size);
            long bytesRead = fread(tempStore->input,chunk_size,1,stdin);
            printf("byts read %d\n",bytesRead);
            if(bytesRead == 0) { break;}
            tempStore->inputsize = chunk_size;
            tempStore->key = malloc(sizeof(char) * chunk_size);
            memcpy(tempStore->key,keybuffer,chunk_size);

            tempStore->output = malloc(sizeof(char)* bytesRead);
            tempStore->thread_id=spawned_threads;
            //pthread_create(threads+spawned_threads, NULL, (void *(*)(void *)) getXorOutput, tempStore);
            threadpool_add(pool,getXorOutput,tempStore,0);
            shift_1bits_left(keybuffer,chunk_size);
            spawned_threads++;
        }
        // collect the data for that spawned threads so far
       pthread_mutex_lock(&m);
       while(count<spawned_threads){
           pthread_cond_wait(&cv, &m);
       }
       pthread_mutex_unlock(&m);
       count=0;
       memset(outputbuffer+pos,0, spawned_threads* chunk_size);
       for(int  j=0;j<spawned_threads;j++){
           memcpy(outputbuffer+pos,inputs[j].output,inputs[j].inputsize);
           pos=pos+inputs[j].inputsize;
       }
       write(outwrite,outputbuffer,32);
       free(inputs);
   }


    close(outwrite);
    return 0;
}

void *getXorOutput(thread_input *threadinput){
     for(int i=0;i < (threadinput->inputsize) ; i++){
            threadinput->output[i] = threadinput->input[i] ^ (threadinput->key[i]);
     }

    pthread_mutex_lock(&m);
    count++;
    pthread_cond_signal(&cv);
    pthread_mutex_unlock(&m);
}

void shift_1bits_left(uint8_t* array, long size)
{
    int i;
    uint8_t shifted = 0x00;
    uint8_t overflow = (array[0] >>7) & 0x1;
    for (i = (size - 1); i>=0 ; i--)
    {
        //printf("i value %d\t",i);
        shifted = (array[i] << 1) | overflow;
        overflow = (array[i]>>7) & 0x1;
        array[i] = shifted;

    }
}



