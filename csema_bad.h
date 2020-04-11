/**
 * Counting semaphores header file* 
 */
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef CSEMA_BAD_H
#define CSEMA_BAD_H



typedef struct {
    int csem_val;
    sem_t wait; // Block this semaphore
    sem_t mutex; // protect value
} csema_t; // Struct for counting semaphore


void csema_wait(csema_t *csema);

void csema_post(csema_t *csema);

void csema_init(csema_t *sem, int max_proc);

void csema_destroy(csema_t *csema);

#endif