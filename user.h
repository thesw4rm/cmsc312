//
// Created by ytpillai on 3/20/20.
//

#ifndef A3_USER_H
#define A3_USER_H
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "constants.h"
typedef struct {
    int *user_procs_left; // Decrement when user thread completes or quit if suddenly set to zero
} user_params;

void *user(void *thread_n);
#endif //A3_USER_H
