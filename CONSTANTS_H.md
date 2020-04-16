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


`DEFAULT_UPROCS` and `DEFAULT_PPROCS` are for when command line arguments are
not provided. 

`PRINT_QUEUE_LIMIT` specifies the maximum size of the print queue. (For now the
assignment is hardcoded to use 30.)

`DEFAULT_SHM_SIZE` and `DEFAULT_PQUEUE_SIZE` refer to the bytes size of the printer
queue metadata and size of the printer queue, respectively.

<br/><br/>

## Printer Queue Metadata

### Code Referred To

```c
typedef struct {
  int user_procs_left; // Number of user processes left to execute (set directly
                       // to 0 in case of sigint)
  job *head;           // Offset to get to the first item in print queue
  sem_t pqmut[30];     // Mutexes for print queue
  sem_t qmut;          // Mutex for metadata
  csema_t qempty;      // Semaphore for when queue has no print jobs
  csema_t qfull;       // Semaphore for when print queue is full
  csema_t uprocs_comp; // Post when all user processes are complete
  csema_t pprocs_comp;

} shm_mdata;
```

### Explanation

The comments in the code explain most of what the metadata means.

`user_procs_left` is the number of user processes not completed yet. 

`head` refers to the first job in the printer queue, `pqmut` is an array of
semaphores for each position in the printer queue, and `qmut` is a semaphore
that acts as a mutex for the metadata itself.

`qempty` and `qfull` are
semaphores for checking if the queue is empty or full, and
`uprocs_comp`/`pprocs_comp` are for determining when the user forks and printer
threads have completed their jobs. 
