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
#include "thpool.h"
//#include "sharedbuffer.h"
#include "encrypt.h"
#include "mypool.h"





int outwrite;

shared_buffer *sharedbuff;


static const char *HELP_MESSAGE = " The usage encrypt -k keyfile.bin -n 1 < plain.bin > cypher.bin";
const int CHAR_SIZE= sizeof(char);
static const int BUFFER_CONSTANT =4;


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
                //printf("%s",HELP_MESSAGE);
                break;
            case 'v' :
                verbose =true;
                break;
            default:
                printf("%s",HELP_MESSAGE);
        }
    }


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


   // threadpool pool = thpool_init(NUM_THREADS);
    tpool *pool = threadpool_init(NUM_THREADS,10);
    outwrite = open("/afs/andrew.cmu.edu/usr19/hghanta/apple2/output2",O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    //create a consumer thread and wait for the output buffer to get filled
    pthread_t outputThread;
    pthread_create(&outputThread, NULL, (void *(*)(void *)) outputData, outputbuffer);

    sharedbuff = malloc(sizeof(shared_buffer));
    sharedbuffer_init(sharedbuff,10);

    while(!feof(stdin)) {
        char *inputbuffer = malloc(sizeof(char) * BUFFER_SIZE);
        long bytes_in_buffer= fread(inputbuffer,CHAR_SIZE,BUFFER_SIZE,stdin);
        if(bytes_in_buffer==0) {
            //printf("read 0 byts \n");
            break;
        }


        long processed_bytes=0;

        // Partition the bytes just read into chunk size units
        while(processed_bytes < bytes_in_buffer){
            thread_input *tempStore = malloc(sizeof(thread_input));
            // map the pointers inside each corresponding chunk to the appropriate offset in the buffer
            // handlel the edge case where there are no enough bytes to process that are of chunk size
            if(processed_bytes+chunk_size < bytes_in_buffer) {
                tempStore->input=  inputbuffer+processed_bytes;
                tempStore->inputsize = chunk_size;
                processed_bytes=processed_bytes+chunk_size;
            }else{
                long left_over = bytes_in_buffer-processed_bytes;
                tempStore->input= inputbuffer+processed_bytes;
                tempStore->inputsize = left_over;
                processed_bytes += left_over;
            }
            tempStore->key = malloc(sizeof(char) * chunk_size);
            memcpy(tempStore->key,keybuffer,chunk_size);
            //thpool_add_work(pool,(void (*)(void *)) getXorOutput, tempStore);
            threadpool_add_work(pool,(void (*)(void *)) getXorOutput, tempStore,false);
            left_shift_key(keybuffer, chunk_size);
        }

    //thpool_wait(pool);
    threadpool_wait(pool);
    outpack *out = malloc(sizeof(outpack));
    out->buffer=inputbuffer;
    out->end=false;
    out->size=processed_bytes;
    sharebuffer_insert(sharedbuff,out);

    }
    outpack *endsignal = malloc(sizeof(outpack));
    endsignal->size=0;
    endsignal->end=true;
    sharebuffer_insert(sharedbuff,endsignal);
    pthread_join(outputThread,NULL);
    return 0;
}

void *getXorOutput(thread_input *tip){
    int i;
    for( i=0;i < (tip->inputsize) ; i++){
        tip->input[i] = tip->input[i] ^ (tip->key[i]);
    }

}

void outputData() {
    while (1) {
        outpack *output = sharedbuffer_remove(sharedbuff);
        if(output->end){
            break;
        }
        write(outwrite, output->buffer, output->size);
        free(output);
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



