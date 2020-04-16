# dll.c

## init_fcfs_job

### Code Referred To

```c
void init_fcfs_job(job *njob, int bytes, unsigned int off) {
    njob->add_job = add_job_fcfs;
    njob->rm_job = rm_job;
    njob->bytes = bytes;
    njob->next = NULL;
}
```

### Explanation

Creates job that will be used in first come first serve. 

<br/><br/>

## add_job_fcfs

### Code Referred To

```c
/**
 * Adds job to end of list (First Come First Serve)
 * @param cjob - pointer to current job list
 * @param njob - pointer to new job
 */

struct print_job *add_job_fcfs(struct print_job *cjob, struct print_job *njob) {
    if(cjob == NULL){ // Adding to empty queue
        return njob;
    }
    struct print_job  *hoff = cjob;
    while (cjob->next != NULL) {
        cjob = cjob->next;
    }

    cjob->next = njob;


    return hoff;

}

```

### Explanation

Adds a job to the end of the printer queue.


## rm_job

### Code Referred To

```c

struct print_job *rm_job(struct print_job *rjob) {
    return rjob->next;
}

```

### Explanation

Removes a job and returns the next element in the queue. 
