# dll.h

## struct print_job

### Code Referred To

```c
typedef struct print_job {
    struct print_job *next;
    unsigned int thd,
        bytes, // Thread that assigned the job, and the bytes allocated
        f;     // 0 if slot is not free, 1 if slot is free
    clock_t start_wait_time; // Total time job has been waiting

    struct print_job *(*add_job)(struct print_job *, struct print_job *);
    struct print_job *(*rm_job)(struct print_job *);

} job;

```

### Explanation

All explanations are in the comments. Last two members are function pointers to
methods that add or remove a job from the queue. 
