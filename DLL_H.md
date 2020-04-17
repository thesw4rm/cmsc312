# dll.h

## struct print_job

### Code Referred To

```c

typedef struct print_job {
    int next; // Offset of next job
    int off;  // Offset of current job
    unsigned int thd,
        bytes; // Thread that assigned the job, and the bytes allocated
    clock_t start_wait_time; // Total time job has been waiting

    int (*add_job)(struct print_job *, struct print_job *);
    unsigned int (*rm_job)(struct print_job *);
} job;

```

### Explanation

All explanations are in the comments. Last two members are function pointers to
methods that add or remove a job from the queue. 
