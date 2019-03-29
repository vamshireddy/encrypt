

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
#define TESTING

typedef struct _data_buf
{
   uint8_t *buffer;
   uint32_t buffer_size;
   uint8_t *correspond_key;
   bool is_free;
   bool is_encrypt;
   uint32_t token_id;
}data_buf_t;

queue <int> input_queue;
#define FILE_NAME_SIZE 256
pthread_mutex_t lock; 
data_buf_t **arr_pointers ;

void shift_1bits_left(uint8_t* array, long size);
void getXorOutput(uint8_t* buffer1, uint8_t* buffer2, uint32_t size);
void* thread_function(void *unused);
static const char * HELP_MESSAGE = " The usage encrypt -k keyfile.bin -n 1 < plain.bin > cypher.bin";



int main(int argc, char **argv) {

    int i ;
    char key_file_name[FILE_NAME_SIZE];
    uint8_t *key ;
    FILE *key_file;
    data_buf_t *data = NULL;
    pthread_t *thread_ids =NULL;
    uint32_t no_of_bytes_read =0,num_of_threads=0;
    uint32_t MAX_QUEUE_SIZE, MAX_BUFFERS;
    uint32_t key_length;
    uint32_t current_token_id =0 ,last_assigned_token_id =0;
    bool verbose=false;
    bool input_done =false;
    bool found_free_buf =false;

#ifndef TESTING
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

#else
    num_of_threads = 4;
    char input_data[] = {0xFF,0xFF};
    int keyfile = open("/home/raram/keyfile",O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    for(int i =0;i <1;i++){
        write(keyfile,input_data,2);
    }
    int no_of_itr =0;
    char input_data1[] = {0x1,0x2,0x3,0x4,0x11,0x12,0x13,0x14,0x1,0x2,0x3,0x4,0x11,0x12,0x13,0x14,0x1,0x2,0x3,0x4,0x11,0x12,0x13,0x14};
    int inputfile = open("/home/raram/input",O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    write(inputfile,input_data1,24);
    close(inputfile);
    memset(key_file_name,0, FILE_NAME_SIZE);
    memcpy(key_file_name,"/home/raram/keyfile",FILE_NAME_SIZE );    
#endif
   

    if(verbose){
        printf("verbose output is on\n");
        printf("Key file name is %s\n",key_file_name);
        printf("Number of threads are %ld\n",num_of_threads);
    }
   key_file = fopen(key_file_name,"rb");
   fseek(key_file,0L,SEEK_END);
   key_length = ftell(key_file);
   rewind(key_file);
   key= (uint8_t*)malloc(key_length);
   fgets((char*)key, key_length, key_file);
   fclose(key_file);
   printf("key data %0X\n",key);
   memcpy(key,input_data, key_length);
   printf("key data %0X\n",key);
  
  /* why 3 ?
    2 sets-input buffers, 1- set for output buffers
     some of the buffers might have been processed and waiting for them to 
      be printed to stdout, even when the queue size is not full we may not find the buffers
      to read and fll the data as we have fixed no of buffers */ 
  MAX_QUEUE_SIZE = num_of_threads *2 ;
  MAX_BUFFERS = MAX_QUEUE_SIZE+num_of_threads;
 
  arr_pointers = (data_buf_t **)malloc(sizeof(uint32_t) *MAX_BUFFERS);
  
  /* Allocate the buffers */
  /* why allocating fixed no of buffers upfront instead allocating them on the fly ?
     as memory allocation funtions are costly operation it might be efficieent to 
     allocate a fixed set of buffers and use them when needed */
  for(i =0 ; i < MAX_BUFFERS ; i++)
  {
    
    data = (data_buf_t *)malloc(sizeof(data_buf_t));
    data->buffer = (uint8_t*)malloc(sizeof(uint8_t)*key_length);
    data->correspond_key = (uint8_t*)malloc(sizeof(uint8_t)*key_length);
    data->is_free = true;
    data->is_encrypt =false;
    arr_pointers[i] = data ;
  }

  if (pthread_mutex_init(&lock, NULL) != 0) 
  { 
        fprintf(stderr,"\n mutex init has failed\n"); 
        return -1; 
   }
  thread_ids = (pthread_t *)malloc(num_of_threads *sizeof(pthread_t));
  for(i=0 ; i< num_of_threads ; i++)
  {  
     /* create threads*/
     pthread_create(&thread_ids[i], NULL, thread_function,NULL );

   }
  
  printf("no of threads %d , key_length %d \n",num_of_threads,key_length);
  /* read the data and fill the buffers */  
  inputfile = open("/home/raram/input", O_RDONLY);
  do
  {

      if(input_done ==false)
      {
         printf("queue size %d\n",input_queue.size());
         if(input_queue.size() < MAX_QUEUE_SIZE)
         {
            found_free_buf =false;
            for(i = 0 ;i < MAX_BUFFERS ; i++)
            {
               data = arr_pointers[i];
               if(data->is_free ==true)
               {  
                 found_free_buf = true;
                 break;
               }
            }  
       
            if(found_free_buf == true)
            {
               printf("found free buf \n");
               data->is_free = false;
               memset(data->buffer,0,key_length);
#ifndef TESTING
               no_of_bytes_read = read(STDIN_FILENO, data->buffer,key_length);
#else
               no_of_bytes_read = read(inputfile,data->buffer,key_length);
               printf(" no_of_bytes_read %d \n",no_of_bytes_read);
              // close(inputfile);
              // memcpy(data->buffer,input_data1+(no_of_itr*key_length),key_length);
#endif
               printf(" no_of_bytes_read %d \n",no_of_bytes_read);
               printf("data read main thread%s \n",data->buffer);
               if(no_of_bytes_read > 0)
               {
                   data->token_id = last_assigned_token_id++;
                   data->buffer_size = no_of_bytes_read;
                   memcpy(data->correspond_key, key , key_length);
                   shift_1bits_left(key,key_length);
                   data->is_encrypt =false;
                   input_queue.push(i);
               }
               if(no_of_bytes_read < key_length)
               {
                  input_done = true;
               } 
             }   
          }   
      } 
      for(i=0; i<MAX_BUFFERS; i++)
      {
         data = arr_pointers[i];
         if((data->is_free == false) &&(data->is_encrypt ==true) && 
            (data->token_id == current_token_id))
         { 
            printf("data printing corresponding token id %d\n",current_token_id);
             current_token_id ++;
#ifndef TESTING
            fprintf(stdout,"%*s",data->buffer_size,data->buffer);
#else
            printf("data after encryption %s\n",data->buffer);
#endif
             data->is_free = true;
          }
      }
   printf("no of iteration %d\n",no_of_itr++);

  }while((current_token_id != last_assigned_token_id)&& (!(input_done)));
  
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
    int index =0;

    while(1)
    {
      
      pthread_mutex_lock(&lock);
      if(!(input_queue.empty()) )
      {
         index =  input_queue.front();
         input_queue.pop();
         printf(" processing the data at index %d thread id %0X\n",index,pthread_self());
         data = arr_pointers[index];
         printf("data is %s\n ",data->buffer);
         is_data_available = true;
      }
      pthread_mutex_unlock(&lock); 
 
      if(is_data_available == true)
      {
        getXorOutput(data->buffer, data->correspond_key, data->buffer_size); 
        data->is_encrypt =true;
        is_data_available = false;
      }
    }
}      

void getXorOutput(uint8_t* buffer1, uint8_t* buffer2, uint32_t size){
     for(int i=0;i <size ; i++){
            buffer1[i] = buffer1[i] ^ buffer2[i];
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

