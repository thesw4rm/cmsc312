# main.c

## Global Variables


### Code referred to
```c
shm_mdata *mdata;
job *pq;

volatile int sigint_tripped; // Becomes 1 if signal interrupt is received.
pid_t *uprocs;
pthread_t *ptds;
```
### Explanation

All vars are allocated in main method. 

`mdata` and `pq` are shared memory pointers that refer to printer queue
metadata and printer queue, respectively. Because of how `mmap` works, we can
use them in both user forks and printer threads. 

`sigint_tripped` is used by user processes to keep track of SIGINTs that are
being handled. 

`uprocs` and `ptds` are allocated in both main
method and forks, but immediately freed in the forks. The purpose of not
putting these vars in shared memory was to avoid having to lock shared
memory in the signal handler. They are used by main method to kill user forks
in case of SIGINT and to cancel printer threads. 

<br/><br/>


## Signal Handlers

### Code referred to

```c
void main_sigint_handler(int sig_num) {
        for (int i = 0; i < num_uprocs; i++) {
                kill(uprocs[i], SIGINT);
        }
        for(int i = 0; i < num_uprocs; i++){

                int status;
                waitpid(uprocs[i], &status, 0);
        }
}
void sigint_handler(int sig_num) { sigint_tripped = 1; }
```

### Explanation

`main_sigint_handler` is used by the main method as a signal handler. It send
SIGINT to all user processes and lets main method continue as normal. 

`sigint_handler` is used by user processes. It sets the user's copy of
`sigint_tripped` to 1 so it can be handled by user process at the right time. 

<br/><br/>


## Free Space finders/allocator/deallocator

### Code referred to

```c
unsigned int get_next_free_off(unsigned int free_space) {
    unsigned int noff = 0;
    while ((free_space & (1 << noff)) == 0) {
        ++noff;
    }
    // print_bits(free_space);
    return noff;
}

unsigned int set_off_not_free(unsigned int free_space, unsigned int off) {
    unsigned int mask = 0x3fffffff ^ (1 << off); // Mask is for 30 bits
    return free_space & mask;
}

unsigned int set_off_free(unsigned int free_space, unsigned int off) {
    unsigned int mask = 1 << off;
    return free_space | mask;
}

```

### Explanation

See explanation of `free_space` variable in `CONSTANTS.H.md`
before reading this section.

`get_next_free_off` finds the next bit that is 1 in `free_space`. It uses `noff` to create a bitmask that, when ANDed with `free_space`, will return the freeness of the print job at `noff` index. It loops until a free slot is found, otherwise returns `noff` as out of range. 


`set_off_not_free` will set the bit at given offset to 0. Essentially, it is marked as not free.

`set_off_free` will set the bit at given offset to 1. Essentially, it is marked as free.

<br/><br/>

## User processes

### Code Referred To - Begin

```c
signal(SIGINT, sigint_handler);
srand(seed);

int num_jobs = rand() % 30 + 1;
printf("User %d is creating %d jobs\n", tn, num_jobs);

```

### Explanation

1. Set the signal handler
2. Seed random numbers using the randomly generated seed from the main method
3. Define number of jobs this user will create
4. Inform the user has begun execution


### Code Referred To - Loop

```c
    for (i = 0; i < num_jobs; i++) {
        usleep(rand() % 900000 + 100000);

        csema_wait(&md->qfull);
        pthread_mutex_lock(&md->qmut);
        int bytes = rand() % 900 + 100;
        unsigned int noff = get_next_free_off(md->free_space);
        job *njob = &pqueue[noff];
        init_sjf_job(njob, bytes, noff);

        njob->start_wait_time = clock();
        njob->thd = tn;
        njob->off = noff;
        if (md->head >= 0) {
            md->head = add_job_sjf(&pqueue[md->head], njob);
        } else {
            md->head = njob->off;
        }

        md->free_space = set_off_not_free(md->free_space, noff);

        pthread_mutex_unlock(&md->qmut);
        csema_post(&md->qempty);

        printf("User %d created job of %d bytes\n", tn, bytes);
        if (sigint_tripped == 1) {

            printf("USER %d received SIGINT. Stopping execution at %d jobs\n",
                   tn, i + 1);
            break;
        }
    }
```


### Explanation

1. Loop through random jobs
2. Sleep for random amount of time between creating each job (to help with
   testing)
3. Wait until the printer queue is not full
4. Define job size (random number of bytes)
5. Get next free index
6. Initialize new SJF job in found location with user thread and start time
7. Lock metadata mutex and add to printer queue LinkedList
        1. Replace head with result of add function in case new job is the shortest job
        2. Replace head with current job if it is -1 (means queue is empty)
8. Set job location to not free
9. Unlock metadata mutex
11. Increment empty semaphore because new job was added
12. If signal interrupt was tripped, only now break out of the loop (user
    processes are hence atomic)


### Code Referred To - End


```c
pthread_mutex_lock(&md->qmut);
md->user_procs_left--;
if (md->user_procs_left == 0) {
        csema_post(&md->uprocs_comp);
}
free(ptds);
free(uprocs);
pthread_mutex_unlock(&md->qmut);
exit(0);
printf("USER PROC %u IS FINISHED\n", tn);
```

### Explanation
1. Lock metadata
2. Decrement user processes left, inform if all users finished
3. Unlock metadata and exit



<br/><br/>

## Printer Threads

### Code Referred To

```c

void *print(void *thread_n) {
    int tn = *(int *)thread_n; // Thread number
    int shm_fd = shmget(SHM_KEY, DEFAULT_SHM_SIZE, IPC_CREAT | 0777);
    int pqueue_fd = shmget(PQUEUE_SIZE, DEFAULT_PQUEUE_SIZE, IPC_CREAT | 0777);
    shm_mdata *md = shmat(shm_fd, NULL, 0);
    job *pqueue = shmat(pqueue_fd, NULL, 0);
    printf("Printer %d started %d\n", tn, md->user_procs_left);
    job *compl_job; // Store completed job

    while (1) {

        csema_wait(&md->qempty);
        /*if (!(mdata->user_procs_left > 0 || mdata->qhead != NULL))
            break;*/
        pthread_mutex_lock(&md->qmut);
        if (md->head == -1 && md->user_procs_left == 0) {
            csema_post(&md->pprocs_comp);
        }

        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        compl_job = &pqueue[md->head];

        long wait = compl_job->bytes * 5 * 1000;
        unsigned int bytes = compl_job->bytes;
        unsigned int thd = compl_job->thd;
        unsigned int off = compl_job->off;
        md->head = rm_job(compl_job);
        mdata->free_space = set_off_free(mdata->free_space, off);
        if (md->head == -1 && md->user_procs_left == 0) {
            csema_post(&md->pprocs_comp);
        }
        pthread_mutex_unlock(&md->qmut);

        csema_post(&md->qfull);
        usleep(wait);
        clock_t cur_time;
        cur_time = clock();
        printf("Printer %d printed job of %d bytes created by user %d\n", tn,
               bytes, thd);

        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    }
}
```

### Explanation

1. Inform Printer has started, connect to shared memory
2. Create pointer for next job to complete
3. Loop until thread is canceled
    1. Wait until queue is not empty
    2. Lock mutex
    3. Prevent cancellation at this point to protect atomicity
    4. Store wait time, job bytes, and user that created job
    5. Remove job from list and set location as free
    6. If no other jobs are available and user processes are complete,
       inform that printer threads are done
    7. Unlock Mutex
    8. Inform that printer queue is not full
    9. Seconds waited = number of bytes / 200
    10. Record total wait time for later log collection
    11. Allow cancellation because job is complete

## Main Method

### Code Referred To - Init Variables

```c

    signal(SIGINT, main_sigint_handler);
    int user_procs, print_procs;
    if (argc < 3) {
        user_procs = DEFAULT_UPROCS;
        print_procs = DEFAULT_PPROCS;
    } else {
        user_procs = (int)strtol(argv[1], (char **)NULL, 10);
        print_procs = (int)strtol(argv[2], (char **)NULL, 10);
    }
    num_uprocs = user_procs;
    size_t tot_size = 0;
    int shm_fd = shmget(SHM_KEY, DEFAULT_SHM_SIZE, IPC_CREAT | 0777);
    int pqueue_fd = shmget(PQUEUE_SIZE, DEFAULT_PQUEUE_SIZE, IPC_CREAT | 0777);
    mdata = shmat(shm_fd, NULL, 0);
    pq = shmat(pqueue_fd, NULL, 0);
    mdata->user_procs_left = user_procs;
    csema_init(&mdata->qempty, 0);
    csema_init(&mdata->qfull, PRINT_QUEUE_LIMIT);
    csema_init(&mdata->uprocs_comp, 0);
    csema_init(&mdata->pprocs_comp, 0);
    mdata->free_space = 0x3fffffff; // Set all bits to 1
    mdata->head = -1;
    pthread_mutex_init(&mdata->qmut, NULL);
    ptds = malloc(sizeof(pthread_t) * print_procs);
    uprocs = malloc(sizeof(pid_t) * user_procs);
    //    pthread_t utds[user_procs];
    int ptd_num[print_procs];
    int utd_num[user_procs];
    int i = 0;
```

### Explanation

1. Set signal handler as main method signal handler
2. Define user/print processes as defaults if cmd arguments not available,
   otherwise define as per cmd arguments
3. Create metadata via shmget
4. Initialize all custom semaphores
5. Initialize head as -1 (empty queue)
6. Set all jobs as free
8. Allocate pthread and pid object holders
9. Create arrays to pass information about thread/fork number


### Code Referred To - Execution

```c
srand(clock());
while (i < user_procs || i < print_procs) {
        pid_t pid;
        if (i < user_procs) {
            pid = fork();
            int seed = rand();
            if (pid == 0) {
                free(uprocs);
                free(ptds);
                user(i, seed);
                exit(0);
            }
            else {
                uprocs[i] = pid;
            }
        }

        if (i < print_procs) {
            ptd_num[i] = i;
            pthread_create(&ptds[i], NULL, print, &ptd_num[i]);
        }
        i++;
}

csema_wait(&mdata->uprocs_comp);
printf("USER PROCS DONE\n");
csema_wait(&mdata->pprocs_comp);

```

### Explanation

1. Loop until all user/printer processes/threads are created
    1. User
        1. Create forked process and random seed value
        2. Free unused variables
        3. Start user process
        4. Set pid in pid array for signal handler
    2. Printer
        1. Create printer thread
        2. Set pthread object in pthread array and number in `ptd_num`
2. Wait until everything is done

### Code Referred To - Cleanup

```c
for (i = 0; i < print_procs; i++) {

        pthread_cancel(ptds[i]);
}

for (i = 0; i < print_procs; i++) {
        pthread_join(ptds[i], NULL);
}

pthread_mutex_destroy(&mdata->qmut);
csema_destroy(&mdata->qempty);
csema_destroy(&mdata->qfull);
csema_destroy(&mdata->uprocs_comp);
csema_destroy(&mdata->pprocs_comp);
shmdt(mdata);
if (shmctl(shm_fd, IPC_RMID, NULL) < 0)
        perror("shmctl");
if (shmctl(pqueue_fd, IPC_RMID, NULL) < 0)
        perror("shmctl");
free(uprocs);
free(ptds);

```


### Explanation

1. Cancel all printers
2. Wait for all printers to complete their tasks
3. Destroy metadata mutex
4. Destroy all custom semaphores
6. Deallocate shared memory
7. Free pid/pthread_t arrays
