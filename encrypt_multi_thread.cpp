

#include <iostream> 
#include <queue> 
#include <stdio.h>
#include <stdint.h>
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
#include <pthread.h>


using namespace std;



typedef struct _data_buf
{
   uint8_t *buffer;
   uint32_t buffer_size;
   uint8_t *correspond_key;
   bool is_free;
   bool is_encrypt;
   uint32_t token_id;
}data_buf_t;

queue <data_buf_t*> input_queue;

#define FILE_NAME_SIZE 256
int MAX_QUEUE_SIZE ;
int MAX_BUFFERS;
uint32_t key_length;
uint32_t current_token_id =0 ;
uint32_t last_assigned_token_id =0;

data_buf_t **arr_pointers ;

void shift_1bits_left(uint8_t* array, long size);
void getXorOutput(uint8_t* buffer1, uint8_t* buffer2, uint32_t size);
void* thread_function(void *unused);
static const char * HELP_MESSAGE = " The usage encrypt -k keyfile.bin -n 1 < plain.bin > cypher.bin";



int main(int argc, char **argv) {

    int c,i ;
    char key_file_name[FILE_NAME_SIZE];
    uint8_t *key ;
    bool verbose=false;
	uint32_t num_of_threads =0;
	uint32_t key_length =0;
	FILE *key_file;
	data_buf_t *data = NULL;
	pthread_t *thread_ids;
	uint32_t no_of_bytes_read =0;
	bool input_done =false;
	bool found_free_buf =false;

#if 0
    while ((c = getopt(argc, argv, "k:N:d")) != EOF) {
        switch (c) {
            case 'N':           // Take number of threads as the input
                num_of_threads = strtol(optarg,&temp,10);
                break;
            case 'k':           // key file
                key_file = malloc(sizeof(char)*strlen(optarg));
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
#endif

    for(i = 1 ; i < argc ; i++)
   {
      if(strncmp(argv[i] ,"-k", sizeof("-k")) == 0)
      {
	      strncpy(key_file_name,argv[i+1],sizeof(argv[i+1]));
		  i = i+1;
	  }
	  else if(strncmp(argv[i] ,"-n", sizeof("-n")) == 0)
	  {
	     num_of_threads = atoi(argv[i+1]);
         i = i+1;
      }
	  else  
	  {
	     fprintf(stderr ,"unknown command line option %s \n",argv[i]);
		 return -1;
      }		 
   }


    if(verbose){
        printf("verbose output is on\n");
        printf("Key file name is %s\n",key_file_name);
        printf("Number of threads are %ld\n",num_of_threads);
    }
 
   fseek(key_file,0L,SEEK_END);
   key_length = ftell(key_file);
   rewind(key_file);
   key= (uint8_t*)malloc(key_length);
   fgets((char*)key, key_length, key_file);
  
  /* why 3 ?
    2 sets-input buffers, 1- set for output buffers
  	   some of the buffers might have been processed and waiting for them to 
  	   be printed to stdout, even when the queue size is not full we may not find the buffers
  	   to read and fll the data as we have fixed no of buffers */ 
  MAX_QUEUE_SIZE = num_of_threads *2 ;
  MAX_BUFFERS = MAX_QUEUE_SIZE+num_of_threads;
 
  arr_pointers = (data_buf_t **)malloc(sizeof(uint32_t) *MAX_QUEUE_SIZE);
  
  /* Allocate the buffers */
  /* why allocating fixed no of buffers upfront instead allocating them on the fly ?
     as memory allocation funtions are costly operation it might be efficieent to 
     allocate a fixed set of buffers and use them when needed */
  for(i =0 ; i < MAX_BUFFERS ; i++)
  {
    
    data = (data_buf_t *)malloc(sizeof(data_buf_t));
	data->buffer = (uint8_t*)malloc(sizeof(uint8_t)*key_length);
	data->correspond_key = (uint8_t*)malloc(sizeof(uint8_t)*key_length);
    arr_pointers[i] = data ;
  }
    
 
  thread_ids = (pthread_t *)malloc(num_of_threads *sizeof(pthread_t));
  for(i=0 ; i< num_of_threads ; i++)
  {  
     /* create threads*/
	  pthread_create(&thread_ids[i], NULL, thread_function,NULL );
	
   }
  
  /* read the data and fill the buffers */  
  do
  {

      if(input_done ==false)
      {
         if(input_queue.size() < MAX_QUEUE_SIZE)
         {
         	found_free_buf =false;
	        for(i = 0 ;i < MAX_BUFFERS ; i++)
	        {
	          data = arr_pointers[i];
		      if(data->is_free ==true)
		      {  
   		        break;
   		        found_free_buf = true;
   		      }
	       }  
	       
	       data->is_free = false;
	       memset(data->buffer,0,key_length);
	       data->token_id = last_assigned_token_id++;
	       no_of_bytes_read = read(STDIN_FILENO, data->buffer,key_length);
	       data->buffer_size = no_of_bytes_read;
	       memcpy(data->correspond_key, key , key_length);
	       shift_1bits_left(key,key_length);
	       input_queue.push(data);
	       if(no_of_bytes_read < key_length)
	       {
	  	      input_done = true;
	       }    
		 }   
	   } 
      for(i=0; i<MAX_BUFFERS; i++)
      {
    	 data = arr_pointers[i];
	     if((data->is_free == false) &&(data->is_encrypt ==true) && 
	          (data->token_id == current_token_id))
          {
		     current_token_id ++;
		     fprintf(stdout,(char*)data->buffer);
             data->is_free = true;
		
          }
      }
      if(input_done ==true)
      {

	  }
  }while((input_done)&&(current_token_id != last_assigned_token_id));
  
  for(i=0; i< num_of_threads; i++)
  {
  	/* by this time all threads should have finishied processing */
  	/* cleanup of mutex lock not required since all threads have finished 
  	   processing */
  	pthread_cancel(thread_ids[i]);
  }
  free(thread_ids);
  for(i=0; i<MAX_BUFFERS; i++)
  {
     data = arr_pointers[i]; 
     free (data->buffer);
     free(data->correspond_key);
	 free(data);
	 arr_pointers[i]= NULL;
  }
  free(arr_pointers);
  free(key);
  
  return 0;
}
 
 
void* thread_function(void * unused)
{
	data_buf_t *data;
	bool is_data_available =false;
	
    while(1)
    {
	/* mutex lock */
	    if(!(input_queue.empty()) )
	    {
	       data =  input_queue.front();
	       input_queue.pop();
	       is_data_available = true;
       }
	/* mutex unlock */
	            		
	   getXorOutput(data->buffer, data->correspond_key, data->buffer_size);
	   data->is_encrypt =true;
	   is_data_available = false;
    }
    return NULL;
}      

void getXorOutput(uint8_t* buffer1, uint8_t* buffer2, uint32_t size){
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

