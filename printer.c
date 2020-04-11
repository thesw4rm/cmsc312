//
// Created by ytpillai on 3/21/20.
//

#include <sys/shm.h>
#include "printer.h"

void *print(void *thread_n){
    int shm_fd_loc = shmget(SHM_KEY, DEFAULT_SHM_SIZE, 0);
    shm_mdata *mdata_loc = shmat(shm_fd_loc, NULL, 0);
    int tn = *(int *)thread_n; // Thread number
    printf("Printer %d started %d\n", tn, mdata_loc->user_procs_left);
    job *compl_job; // Store completed job
    while(mdata_loc->user_procs_left > 0 || mdata_loc->qhead != NULL){
        sem_wait(&mdata_loc->qempty);
        printf("ASD\n");
        pthread_mutex_lock(&mdata_loc->qmut);
        if(mdata_loc->qhead == NULL){
            printf("qhead was NULL when consumer %d was trying to process a new print job\n", tn);
        }
        compl_job = mdata_loc->qhead;
        mdata_loc->qhead = mdata_loc->qhead->next;
        if(mdata_loc->qhead == NULL)
            mdata_loc->qtail = NULL;

        pthread_mutex_unlock(&mdata_loc->qmut);
        sem_post(&mdata_loc->qfull);
        printf("Printer: %d,%d,%d,%ld,%ld\n",
               tn, compl_job->thd, compl_job->bytes,
               compl_job->start_wait_time,
               time(NULL) - compl_job->start_wait_time);
        free(compl_job);
    }
    printf("WE OUT PRINT %d\n", tn);
    pthread_exit(0);
}
