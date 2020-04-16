#ifndef DLL_H
#define DLL_H
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
typedef struct print_job{
    int next; // Offset of next job
    int off; // Offset of current job
   unsigned int thd, bytes; // Thread that assigned the job, and the bytes allocated
   clock_t start_wait_time; // Total time job has been waiting


   int (*add_job)(struct print_job *, struct print_job *);
   unsigned int (*rm_job)(struct print_job *);
} job;

int add_job_sjf(struct print_job *, struct print_job *);
int add_job_fcfs(struct print_job *, struct print_job *);

unsigned int rm_job(struct print_job *);

void init_fcfs_job(job *, int, unsigned int);
void init_sjf_job(job *, int, unsigned int);



#endif