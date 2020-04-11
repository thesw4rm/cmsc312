#include "csema_bad.h"

static int min(int a, int b){
    if(a < b){
        return a;
    }
    else{
        return b;
    }
}

void csema_wait(csema_t *csema){
  //  printf("IS THIS WAITING? %s %s %i\n", is_prod == 1 ? "Producer": "Consumer", is_empty == 1 ? "Empty": "Full", csema->csem_val);
    sem_wait(&csema->wait);
    sem_wait(&csema->mutex);
    csema->csem_val--;
    if(csema->csem_val > 0){
        sem_post(&csema->wait);
    }
    sem_post(&csema->mutex);
    
}

void csema_post(csema_t *csema){
 //   printf("IS THIS POSTING? %s %s %i\n", is_prod == 1 ? "Producer": "Consumer", is_empty == 1 ? "Empty": "Full", csema->csem_val);

    sem_wait(&csema->mutex);

    csema->csem_val++;
    if(csema->csem_val == 1)
        sem_post(&csema->wait);
    sem_post(&csema->mutex);
}



void csema_init(csema_t *sema, int max_proc){
    sema->csem_val = max_proc;
    sem_init(&sema->wait, 1, min(1, max_proc)); // Gate
    sem_init(&sema->mutex, 1, 1);
}

void csema_destroy(csema_t *csema){
    sem_destroy(&csema->wait);
    sem_destroy(&csema->mutex);
    
}