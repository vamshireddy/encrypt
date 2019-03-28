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
    long inputsize;
};



typedef struct tip thread_input;

pthread_cond_t cv;
pthread_mutex_t m;
pthread_cond_t cv2;
pthread_mutex_t m2;
pthread_cond_t cv3;
pthread_mutex_t m3;


bool outputDone=false;
bool inputDone=false;
bool startwaiting=false;

long  outputBufferfill=0;
long  currentInputBufferFill=0;

void left_shift_key(unsigned char *existingKey, long size);
void *getXorOutput(thread_input *threadinput);

static const char *HELP_MESSAGE = " The usage encrypt -k keyfile.bin -n 1 < plain.bin > cypher.bin";
const int CHAR_SIZE= sizeof(char);
static const int BUFFER_CONSTANT =4;
void outputData(char *outputbuffer);

int main(int argc, char **argv) {

    int c ;
    char *keyFile;
    long  NUM_THREADS;
    char *temp = malloc(sizeof(char));
    bool verbose=false;

    while ((c = getopt(argc, argv, "k:N:d")) != EOF) {
        switch (c) {
            case 'N':           // Take number of threads as the input
                NUM_THREADS = strtol(optarg,&temp,10);
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
                printf("%s",HELP_MESSAGE);
        }
    }

    if(verbose){
        printf("verbose output is on\n");
        printf("Key file name is %s\n",keyFile);
        printf("Number of threads are %ld\n",NUM_THREADS);
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




   FILE *keyfile = fopen(keyFile,"rb");
   fseek(keyfile,0L,SEEK_END);
   long chunk_size = ftell(keyfile);
   rewind(keyfile);
   long BUFFER_SIZE = BUFFER_CONSTANT * NUM_THREADS * chunk_size;


    unsigned  char keybuffer[chunk_size];
    int i;
   for(i=0;i<chunk_size;i++){
       keybuffer[i] = fgetc(keyfile);
   }


   char outputbuffer[BUFFER_SIZE];   // final output buffer that all threads output will be written to
   char inputbuffer[BUFFER_SIZE];

   //create a thread pool with NUM_THREADS
   tpool *pool = create_tpool(NUM_THREADS, 4 * NUM_THREADS);
   int outwrite = open("/Users/harsha/CLionProjects/encrypt/output",O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

   //create a consumer thread and wait for the output buffer to get filled
   pthread_t tid;
   pthread_create(&tid,NULL,outputData,outputbuffer);

   while(!feof(stdin)) {

       long bytes_in_buffer= fread(inputbuffer,CHAR_SIZE,BUFFER_SIZE,stdin);
       if(bytes_in_buffer==0) {
           inputDone=true;
           break;

       }

      currentInputBufferFill=0;
       long processed_bytes=0;

        while(processed_bytes < bytes_in_buffer){
            thread_input *tempStore = malloc(sizeof(thread_input));
            tempStore->output = outputbuffer+processed_bytes;
            if(processed_bytes+chunk_size < bytes_in_buffer) {
                tempStore->input = malloc(sizeof(char) * chunk_size);
                tempStore->inputsize = chunk_size;
                memcpy(tempStore->input,inputbuffer+processed_bytes,chunk_size);

                processed_bytes +=chunk_size;
                currentInputBufferFill=currentInputBufferFill+chunk_size;

            }else{
                long left_over = bytes_in_buffer-processed_bytes;
                tempStore->input= malloc(sizeof(char) * (left_over));
                tempStore->inputsize = left_over;
                memcpy(tempStore->input,inputbuffer+processed_bytes,left_over);
                processed_bytes+=left_over;
                currentInputBufferFill=currentInputBufferFill+left_over;
                //inputDone=true;
            }
            tempStore->key = malloc(sizeof(char) * chunk_size);
            memcpy(tempStore->key,keybuffer,chunk_size);
            left_shift_key(keybuffer, chunk_size);

            add_work_to_pool(pool, (void (*)(void *)) getXorOutput, tempStore);
        }

        // signal the consumer (outputData)  thread that the buffer is full and it can start waiting to see if the
        // outuput is completed by worker threads
        pthread_mutex_lock(&m3);
        pthread_cond_signal(&cv3);
        startwaiting=true;
        pthread_mutex_unlock(&m3);

        // wait on signal from Consumer(ouputData, which prints to screen)  that it has finished one iter size of data
        pthread_mutex_lock(&m2);
        while(outputDone != true){
           pthread_cond_wait(&cv2, &m2);
        }
        outputDone=false;
        pthread_mutex_unlock(&m2);

   }
   close(outwrite);
   return 0;
}

void *getXorOutput(thread_input *threadinput){
    int i;
     for( i=0;i < (threadinput->inputsize) ; i++){
            threadinput->output[i] = threadinput->input[i] ^ (threadinput->key[i]);
     }
     pthread_mutex_lock(&m);
    outputBufferfill += threadinput->inputsize;
    pthread_cond_signal(&cv);
    pthread_mutex_unlock(&m);
    //act like a producer and add the output to corresponding buffer location
}

void outputData(char *outputbuffer){

  while(!inputDone) {
      pthread_mutex_lock(&m3);
      while(!startwaiting ){
          pthread_cond_wait(&cv3,&m3);
      }
      pthread_mutex_unlock(&m3);

      pthread_mutex_lock(&m);
      while (outputBufferfill < currentInputBufferFill) {
          pthread_cond_wait(&cv, &m);
      }
      write(fileno(stdout), outputbuffer, outputBufferfill);
      memset(outputbuffer,0,outputBufferfill);
      outputDone=true;
      outputBufferfill = 0;
      pthread_mutex_unlock(&m);

      //signal the main thread to process to fetch/read data from stdin
      pthread_mutex_lock(&m2);
      pthread_cond_signal(&cv2);
      pthread_mutex_unlock(&m2);

  }

}

void left_shift_key(unsigned char *existingKey, long size){

    int i;
    unsigned char shifted ;
    unsigned char overflow = (existingKey[0] >>7) & 0x1;
    for (i = (size - 1); i>=0 ; i--)
    {
        shifted = (existingKey[i] << 1) | overflow;
        overflow = (existingKey[i]>>7) & 0x1;
        existingKey[i] = shifted;


    }
}



