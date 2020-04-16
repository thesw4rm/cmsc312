//
// Created by ytpillai on 3/22/20.
//

#ifndef A3_CONSTANTS_H

#define A3_CONSTANTS_H
#define SHM_KEY 99178
#define PQUEUE_SIZE 99112
#define DEFAULT_UPROCS 10
#define DEFAULT_PPROCS 10
#define PRINT_QUEUE_LIMIT 30




// Printer format: 1, printer_id, user_id, bytes, start_time, wait_time
// User format: 0, -1, user_id, bytes, start_time, 0

#include "dll.h"
#include "csema_bad.h"
/* *
 * Metadata for printer queue
 * */
typedef struct {
    unsigned int sigint_tripped; // Set to 1 if sigint is received
    unsigned int free_space; // Each bit says that index is free for another print job
    int user_procs_left; // Number of user processes left to execute (set directly to 0 in case of sigint)
    int head; // Offset to get to the first item in print queue
    pthread_mutex_t qmut; // Mutex for print queue
    csema_t qempty; // Semaphore for when queue has no print jobs
    csema_t qfull; // Semaphore for when print queue is full
    csema_t uprocs_comp; // Post when all user processes are complete
    csema_t pprocs_comp;

} shm_mdata;


#define DEFAULT_SHM_SIZE sizeof(shm_mdata)
#define DEFAULT_PQUEUE_SIZE sizeof(job) * 30

#define QUEUE_OFFSET sizeof(shm_mdata)


#endif //A3_CONSTANTS_H

