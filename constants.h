//
// Created by ytpillai on 3/22/20.
//

#ifndef A3_CONSTANTS_H

#define A3_CONSTANTS_H
#define SHM_KEY 99178
#define PQUEUE_SIZE 99112
#define DEFAULT_UPROCS 5
#define DEFAULT_PPROCS 10
#define PRINT_QUEUE_LIMIT 30

// Printer format: 1, printer_id, user_id, bytes, start_time, wait_time
// User format: 0, -1, user_id, bytes, start_time, 0

#include "csema_bad.h"
#include "dll.h"

typedef struct {
  int user_procs_left; // Number of user processes left to execute (set directly
                       // to 0 in case of sigint)
  job *head;           // Offset to get to the first item in print queue
  sem_t pqmut[30];     // Mutexes for print queue
  sem_t qmut;          // Mutex for metadata
  csema_t qempty;      // Semaphore for when queue has no print jobs
  csema_t qfull;       // Semaphore for when print queue is full
  csema_t uprocs_comp; // Post when all user processes are complete
  csema_t pprocs_comp;

} shm_mdata;

#define DEFAULT_SHM_SIZE sizeof(shm_mdata)
#define DEFAULT_PQUEUE_SIZE sizeof(job) * 30

#define QUEUE_OFFSET sizeof(shm_mdata)

#endif // A3_CONSTANTS_H

/* if (mdata->qtail == NULL) { // Tail should not be NULL unless this is first
print job (thread and i are 0)
//            if ((tn + i) != 0)
//                printf("qtail was NULL when user %d %d was trying to add a new
print job\n", tn, i); if (mdata->qhead == NULL) mdata->qhead = njob; else {
             mdata->qtail = njob;
             mdata->qtail->prev = mdata->qhead;
             mdata->qhead->next = mdata->qtail;

         }
         //mdata->qtail = njob;

         //printf("%d\n", mdata->qhead->bytes);

     } else {
         if (mdata->qhead == NULL) {
             mdata->qhead = njob;
             mdata->qtail = NULL;

         } else {
             mdata->qtail->next = njob;
             njob->prev = mdata->qtail;
             mdata->qtail = njob;
         }

     }*/