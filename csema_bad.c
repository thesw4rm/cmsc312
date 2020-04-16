#include "csema_bad.h"
void csema_wait(csema_t *csema, int is_prod, int is_empty){
    sem_wait(&csema->mutex);
    csema->csem_val--;
    if(csema->csem_val < 0){
        sem_post(&csema->mutex);
        sem_wait(&csema->wait);
    }
    else{
        sem_post(&csema->mutex);
    }
    
}

void csema_post(csema_t *csema, int is_prod, int is_empty){

    sem_wait(&csema->mutex);
    csema->csem_val++;
    if(csema->csem_val <= 0)
        sem_post(&csema->wait);
    sem_post(&csema->mutex);
}

void csema_init(csema_t *sem, int max_proc){
    sem->csem_val = max_proc;
    sem_init(&sem->wait, 0, 0);
    sem_init(&sem->mutex, 0, 1);
}

void csema_destroy(csema_t *csema){
    sem_destroy(&csema->wait);
    sem_destroy(&csema->mutex);
    
}