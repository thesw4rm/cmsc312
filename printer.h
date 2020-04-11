//
// Created by ytpillai on 3/21/20.
//

#ifndef A3_PRINTER_H
#define A3_PRINTER_H
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "constants.h"

typedef struct {
    int *user_procs_left; // If zero then quit printer thread

} printer_params;

void *print(void *thread_n);
#endif //A3_PRINTER_H
