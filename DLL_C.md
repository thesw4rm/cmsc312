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

int add_job_sjf(struct print_job *cjob, struct print_job *njob) {
    if (njob->bytes < cjob->bytes) {
        njob->next = cjob->off;
        return njob->off;
    }
    int hoff = cjob->off;
    while (cjob->bytes <= njob->bytes && cjob->next != -1) {
        cjob = &cjob[cjob->next - cjob->off]; // Negative array index means go
                                              // backwards that many pointers
    }
    njob->next = cjob->next;

    cjob->next = njob->off;
    return hoff;
}
```

### Explanation

Loops through until a job is found that has more bytes that new job, or last element is reached. If element is head, then set new job next offset to be the current head, and return new job as the new head. Otherwise, insert new job into the queue. 


## rm_job

### Code Referred To

```c

struct print_job *rm_job(struct print_job *rjob) {
    return rjob->next;
}

```

### Explanation

Removes a job and returns the next element in the queue. 
