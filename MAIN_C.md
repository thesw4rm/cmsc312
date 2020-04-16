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

        csema_wait(&mdata->qfull);
        int bytes = rand() % 900 + 100;
        unsigned int noff = 0;
        int free_found = 0;
        while (free_found == 0) {
            if (sem_trywait(&mdata->pqmut[noff]) >= 0) {
                // Try to wait, if it is being used then move on to next slot
                if (pq[noff].f == 0) {
                    // Not free, must keep going
                    sem_post(&mdata->pqmut[noff]);
                    ++noff;
                } else {
                    // We found a free location
                    free_found = 1;
                }
            }
        }
        job *njob = &pq[noff];
        njob->f = 0; // Set as not free
        init_fcfs_job(njob, bytes, noff);

        njob->start_wait_time = clock();
        njob->thd = tn;
        sem_wait(&mdata->qmut);
        if (mdata->head != NULL) {
            add_job_fcfs(mdata->head, njob);
        } else {
            mdata->head = njob;
        }
        sem_post(&mdata->qmut);

        sem_post(&mdata->pqmut[noff]); // Unlock the slot for use by printer
        csema_post(&mdata->qempty);
        printf("User %d created job of %d bytes\n", tn, bytes);
        if(sigint_tripped == 1){
            printf("User %d caught SIGINT after completing %d jobs\n", tn, i+1);
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
5. Loop through all semaphores (mutexes) in `pqmut` until free location is
   found. Then lock that free location
        1. Ignore locations that are not free or locked
6. Initialize new job in found location with user thread and start time
7. Lock metadata mutex and add to printer queue LinkedList
8. Set job location to not free
9. Unlock metadata mutex
10. Unlock free location
11. Increment empty semaphore because new job was added
12. If signal interrupt was tripped, only now break out of the loop (user
    processes are hence atomic)


### Code Referred To - End

```c
    printf("USER PROC %u IS FINISHED\n", tn);
    sem_wait(&mdata->qmut);
    mdata->user_procs_left--;
    if (mdata->user_procs_left == 0) {
        csema_post(&mdata->uprocs_comp);
    }
    sem_post(&mdata->qmut);
    exit(0);

```

### Explanation
1. Inform User process is finished
2. Lock metadata
3. Decrement user processes left
4. Unlock metadata and exit



<br/><br/>

## Printer Threads

### Code Referred To

```c

void *print(void *thread_n) {
    int tn = *(int *)thread_n; // Thread number
    printf("Printer %d started %d\n", tn, mdata->user_procs_left);
    job *compl_job; // Store completed job

    while (1) {

        csema_wait(&mdata->qempty);
        sem_wait(&mdata->qmut);
        if (mdata->head == NULL && mdata->user_procs_left == 0) {
            csema_post(&mdata->pprocs_comp);
        }

        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        // TODO: Subtract the overhead time of this stuff
        compl_job = mdata->head;

        long wait = compl_job->bytes * 5 * 1000;
        unsigned int bytes = compl_job->bytes;
        unsigned int thd = compl_job->thd;
        mdata->head = rm_job(compl_job);
        compl_job->f = 1; // Set job as free because we are printing it
        if (mdata->head == NULL && mdata->user_procs_left == 0) {
            csema_post(&mdata->pprocs_comp);
        }
        sem_post(&mdata->qmut);

        csema_post(&mdata->qfull);
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

1. Inform Printer has started
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
    10. Record total running time for later log collection
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
size_t tot_size = 0;
mdata = mmap(NULL, DEFAULT_SHM_SIZE, PROT_READ | PROT_WRITE,
         MAP_SHARED | MAP_ANONYMOUS, -1, 0);

pq = mmap(NULL, DEFAULT_PQUEUE_SIZE, PROT_READ | PROT_WRITE,
      MAP_SHARED | MAP_ANONYMOUS, -1, 0);
mdata->user_procs_left = user_procs;
csema_init(&mdata->qempty, 0);
csema_init(&mdata->qfull, PRINT_QUEUE_LIMIT);
csema_init(&mdata->uprocs_comp, 0);
csema_init(&mdata->pprocs_comp, 0);

sem_init(&mdata->qmut, 1, 1);
for (int i = 0; i < 30; i++)
        sem_init(&mdata->pqmut[i], 1, 1);
for (int i = 0; i < 30; i++)
        pq[i].f = 1; // Set all jobs as free initially
mdata->head = NULL;
// Do this in case queue size is variable (change qmut to pointer)
// mdata->qmut = mmap(NULL, sizeof(sem_t) * 30, PROT_READ|PROT_WRITE,
// MAP_SHARED|MAP_ANONYMOUS, -1, 0);
uprocs = malloc(sizeof(pid_t) * user_procs);
ptds = malloc(sizeof(pthread_t) * print_procs);
int ptd_num[print_procs];
int utd_num[user_procs];
int i = 0;
```

### Explanation

1. Set signal handler as main method signal handler
2. Define user/print processes as defaults if cmd arguments not available,
   otherwise define as per cmd arguments
3. Create metadata via mmap
4. Initialize all custom semaphores
5. Initialize all printer queue semaphores
6. Set all jobs as free
7. Set head as NULL to indicate empty queue
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
sem_destroy(&mdata->qmut);
csema_destroy(&mdata->qempty);
csema_destroy(&mdata->qfull);
csema_destroy(&mdata->uprocs_comp);
csema_destroy(&mdata->pprocs_comp);
for (int i = 0; i < 30; i++)
        sem_destroy(&mdata->pqmut[i]);
munmap(mdata, DEFAULT_SHM_SIZE);
munmap(pq, DEFAULT_PQUEUE_SIZE);
free(uprocs);
free(ptds);

```


### Explanation

1. Cancel all printers
2. Wait for all printers to complete their tasks
3. Destroy metadata mutex
4. Destroy all custom semaphores
5. Destroy all printer queue semaphores
6. Deallocate shared memory
7. Free pid/pthread_t arrays
