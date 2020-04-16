#ifndef DLL_H
#define DLL_H
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
typedef struct print_job {
    struct print_job *next;
    unsigned int thd,
        bytes, // Thread that assigned the job, and the bytes allocated
        f;     // 0 if slot is not free, 1 if slot is free
    clock_t start_wait_time; // Total time job has been waiting

    struct print_job *(*add_job)(struct print_job *, struct print_job *);
    struct print_job *(*rm_job)(struct print_job *);

} job;

struct print_job *add_job_fcfs(struct print_job *, struct print_job *);

struct print_job *rm_job(struct print_job *);

void init_fcfs_job(job *, int, unsigned int);

#endif