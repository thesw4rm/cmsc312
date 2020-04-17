# constants.h



## Definitions

### Code Referred to

```c
#define A3_CONSTANTS_H
#define SHM_KEY 99178
#define PQUEUE_SIZE 99112
#define DEFAULT_UPROCS 5
#define DEFAULT_PPROCS 10
#define PRINT_QUEUE_LIMIT 30

#define DEFAULT_SHM_SIZE sizeof(shm_mdata)
#define DEFAULT_PQUEUE_SIZE sizeof(job) * 30

#define QUEUE_OFFSET sizeof(shm_mdata)

```

### Explanation


`SHM_KEY` and `PQUEUE_SIZE` (typo in code that was used throughout program, should be `PQUEUE_KEY`) are the integer keys for shared memory allocated via `shmget`. 

`DEFAULT_UPROCS` and `DEFAULT_PPROCS` are for when command line arguments are
not provided. 

`PRINT_QUEUE_LIMIT` specifies the maximum size of the print queue. (For now the assignment is hardcoded to use 30.)

`DEFAULT_SHM_SIZE` and `DEFAULT_PQUEUE_SIZE` refer to the bytes size of the printer queue metadata and size of the printer queue, respectively.

<br/><br/>

## Printer Queue Metadata

### Code Referred To

```c

typedef struct {
    unsigned int sigint_tripped; // Set to 1 if sigint is received
    unsigned int free_space; // Each bit says that index is free for another print job
    int user_procs_left; // Number of user processes left to execute (set directly to 0 in case of sigint)
    int head; // Offset to get to the first item in print queue
    pthread_mutex_t qmut; // Mutex for print queue
    csema_t qempty; // Semaphore for when queue has no print jobs
    csema_t qfull; // Semaphore for when print queue is full
    csema_t uprocs_comp; // Post when all user processes are complete
    csema_t pprocs_comp;

} shm_mdata;

```


### Explanation

The comments in the code explain most of what the metadata means.

`sigint_tripped` is not used in the program. 

`free_space` is an integer where each bit represents whether the print job slot at that index of the printer queue is free or not. For example, let's say the print queue is of size 10. Our free space integer, initially representing every slot as free, would be `0b1111111111`. If a user adds a job to index 5 of the print queue, the free space variable would then be equal to `0b1111011111`. And so on and so forth. Explanations of how the functions work are available in [MAIN_C.md](MAIN_C.md);


`user_procs_left` is the number of user processes not completed yet. 

`head` refers to the first job in the printer queue. It is an integer because it represents the index of the first print job in the current queue.  

`qmut` is a mutex that is used to lock the metadata.

`qempty` and `qfull` are semaphores for checking if the queue is empty or full, and `uprocs_comp`/`pprocs_comp` are for determining when the user forks and printer threads have completed their jobs. 
