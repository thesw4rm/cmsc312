#include "dll.h"

/**
 *
 * @param njob - Creates new job list/queue from existing and allocated pointer
 */
void init_fcfs_job(job *njob, int bytes, unsigned int off) {
    njob->add_job = add_job_fcfs;
    njob->rm_job = rm_job;
    njob->bytes = bytes;
    njob->next = NULL;
}


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


struct print_job *rm_job(struct print_job *rjob) {
    return rjob->next;
}

/**
 *
 * @param rjob - job to be removed
 * Job must be freed outside of the function
 */
