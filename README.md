# Cryptp encryptor utility

    The utility that will perform a simple XOR crypto transform.
    They key will be given as a set of bytes in an external file (raw
    binary format) and the number of bytes in the key will dictate the "chunk
    size".   The plain text data will be given on stdin and the utility must
    break it into chunk-size sections and XOR it against the key and write the
    cypher text it to stdout.  After each chunk is processed the key is
    rotated left by one bit to make a new key.  This means that the key will
    repeat every N chunks, where N is the number of bits in the key.  The
    plain text may not be a multiple of the chunk size in length.  The plain
    text may also be very large, far exceeding the available memory+swap space
    in the system.

    In addition to just performing the algorithm, the utility needs to
    scale so that multiple chunks of plain text can be efficiently processed in
    parallel on multi-core/multi-CPU machines.   As such, a number of threads
    must be created to process chunks of data concurrently.  Regardless of the
    number of threads, the output cypher text must remain the same.
    Any errors/status will come out on stderr.
    
    example : encrypt -k keyfile.bin -n 1 < plain.bin > cypher.bin


#Treee structure 

    mypool.c :
        Simple thread pool module that is capable of spawning threads 
        and maintaining the current state of the pool. Like the number of 
        threads that atre currently working , idle . Making sure 
        all the threads finish their assigned work before exit. 
        
        I developed this to basically help me with thread pool management 
        
     sharedbuffer.c : 
        
        A circular shared buffer that can be shared by two sets 
        of threads to fill the buffer and to consume the items inserted. 
        The buffer follows FIFO discipline . Remove/ insert calls 
        are blocking if the space is not appropriate.
        
      encrypt.c 
        
        Main code that does the actual work. 
        
       tests: 
            The folder contains small tests that I developed to test. 
            It is not exhaustive though.  
            
#validation : 
     1. Memory leaks verification via VALGRIND . To make sure that I am freeing up resources in my code .
     2. Some simple shell script to verify the correctness of the output. 

#Scope for improvements: 
    
    1.  Choosing the read buffer constants ( the amount of data that is read at once form the stream) 
        based on the  undelyugn cache architecture. If we model our input reads
        in such a way that no threads will run into one another cache lines 
        and thus lead to thrashign , we will get maximum performance gain. 
     
     2. More robust error checking can be done . Currently I am 
        doing error checks for the very likely systems calls that mgiht fail 
        But the code should be robust against all failures , so error 
        checking can be improved. 
     
     3. Thread stress testing : I did not do full fledged stress test of the 
        like by spawning very large number of threads that can be done. 
        And we can profile the code accordingly    
        
        
#Author : 
    Harsha varhdna ghanta
    hghanta@andrew.cmu.edu              
        
    




