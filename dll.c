#include "dll.h"

/**
 *
 * @param njob - Creates new job list/queue from existing and allocated pointer
 */
void init_fcfs_job(job *njob, int bytes, unsigned int off) {
    njob->add_job = add_job_fcfs;
    njob->rm_job = rm_job;
    njob->bytes = bytes;
    njob->next = -1;
    njob->off = off;
}

/**
 *
 * @param njob - Creates new job list/queue from existing and allocated pointer
 */
void init_sjf_job(job *njob, int bytes, unsigned int off) {
    njob->add_job = add_job_sjf;
    njob->rm_job = rm_job;
    njob->bytes = bytes;
    njob->off = off;
    njob->next = -1;
}

/**
 * Adds job to end of list (First Come First Serve)
 * @param cjob - pointer to current job list
 * @param njob - pointer to new job
 */

int add_job_fcfs(struct print_job *cjob, struct print_job *njob) {
    if(cjob == NULL){ // Adding to empty queue
//        printf("WAT ITS NULL?\n");
        return njob->off;
    }
    int hoff = cjob->off;
//    printf("\n");
//    printf("WE WERE CALLED? %u\n", cjob->next);
    while (cjob->next != -1) {
//        printf("NUMSTART %d\n", cjob[cjob->next - cjob->off].off);
        cjob = &cjob[cjob->next - cjob->off];
//        printf("%u %u\n", cjob->off, cjob->next);
    }

//    printf("ASDASD\n");
    njob->next = cjob->next;
    cjob->next = njob->off;

    //printf("%u %u\n", hoff - cjob->off, cjob->off);

    return hoff;

}

int add_job_sjf(struct print_job *cjob, struct print_job *njob) {
    if(njob->bytes < cjob->bytes){
        njob->next = cjob->off;
        return njob->off;
    }
    int hoff = cjob->off;
    while (cjob->bytes <= njob->bytes && cjob->next != -1) {
        cjob = &cjob[cjob->next - cjob->off]; // Negative array index means go backwards that many pointers
    }
    njob->next = cjob->next;

    cjob->next = njob->off;
    return hoff;

}

unsigned int rm_job(struct print_job *rjob) {
    if (rjob->next != -1)
        return rjob->next;
    else
        return -1;
}

/**
 *
 * @param rjob - job to be removed
 * Job must be freed outside of the function
 */
