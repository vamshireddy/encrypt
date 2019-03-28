

#include <iostream> 
#include <queue> 
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


using namespace std;


queue <data_buf_t> input_queue;
queue <data_buf_t> output_queue;

typedef struct _data_buf
{
   uint8_t *buffer;
   unit32_t buffer_size;
   unit32_t *correspond_key;
   bool is_free;
   unit32_t token_id;
}data_buf_t;

int MAX_QUEUE_SIZE ;
unit32_t key_length;
unit32_t current_token_id =0 ;
unit32_t last_assigned_token_id =0;

data_buf_t **arr_pointers ;

void shift_1bits_left(uint8_t* array, long size);
void getXorOutput(thread_input *threadinput);


static const char * HELP_MESSAGE = " The usage encrypt -k keyfile.bin -n 1 < plain.bin > cypher.bin";



int main(int argc, char **argv) {

    int c,i ;
    char *keyFile;
    long  numT;
    char *temp;
    bool verbose=false;
	unit32_t num_of_threads =0;
	unit32_t key_length =0;

    while ((c = getopt(argc, argv, "k:N:d")) != EOF) {
        switch (c) {
            case 'N':           // Take number of threads as the input
                num_of_threads = strtol(optarg,&temp,10);
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
 
   fseek(keyfile,0L,SEEK_END);
   key_length = ftell(keyfile);
   rewind(keyfile);
  
  MAX_QUEUE_SIZE = num_of_threads *3 ;
 
  pointers = (data_buf_t **)malloc(sizeof(unit32_t) *MAX_QUEUE_SIZE)
  
  /* Allocate the buffers */
  for(i =0 ; i < MAX_QUEUE_SIZE ; i++)
  {
    
    data = (data_buf_t *)malloc(sizeof(data_buf_t));
	data->buffer = (uint8_t*)malloc(sizeof(uint8_t)*key_length);
	data->correspond_key = (uint8_t*)malloc(sizeof(uint8_t)*key_length);
    pointers[i] = &data ;
  }
    
 
  for(i=0 ; i< num_of_threads ; i++)
  {  
     /* create thereads*/
	  pthread_create(&thread_ids[i], NULL, thread_function,NULL );
	
   }
  
  /* read the data and fill the buffers */  
  while(1)
  {
      if(input_queue.size() < MAX_QUEUE_SIZE)
      {
	  for(i = 0 i < MAX_QUEUE_SIZE ; i++)
	  {
	       data = pointers[i];
		if(data->is_free ==true)
		{ 
   		   break;
		}
	  }
	  data->is_free = false;
	  memset(data->buffer,0,key_length);
	  data->token_id = last_assigned_token_id++;
	  no_of_bytes_read = read(STDIN_FILENO, data->buffer,key_length);
	  data->buffer_size = no_of_bytes_read;
	  memcpy(data->correspond_key, key , key_length);
	  shift_1bits_left(key,key_length);
	  input_queue.push();
      }
      for(i=0; i<MAX_QUEUE_SIZE; i++)
      {
    	  data = pointers[i];	 
	      if((data->is_encrypt ==true) && (data->token_id == current_token_id))
          {
		current_token_id ++;
		fprintf(stdout,data->buffer);
                data->is_free = true;
          }
      }		  
  }
  
  return 0;
}
 
 
void thread_function()
{
    while(1)
    {
	/* mutex lock */
	if(!(input_queue.isEmpty) )
	{
	   data =  input_queue.first();
	   input_queue.Pop();
	}
	/* mutex unlock */ 
            		
	getXorOutput(data->buffer, data->correspond_key, data->buffer_size);
	data->is_encrypt =true;
    }
}      

void getXorOutput(uint8_t* buffer1, uint8_t* buffer2, unit32_t size){
     for(int i=0;i <size ; i++){
            buffer1[i] = buffer1[i] ^ buffer2[1];
     }
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

