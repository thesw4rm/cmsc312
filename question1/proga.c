#include <stdio.h>
#include <stdint.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include "part1.h"


#define SHM_STR_KEY 4156
#define SHM_INT_KEY 4157

int main(){
    int shm_str = shmget(SHM_STR_KEY, sizeof(mem_string), IPC_CREAT | 0777);
    int shm_int = shmget(SHM_INT_KEY, sizeof(mem_int), IPC_CREAT | 0777);
    
    mem_string *str = shmat(shm_str, NULL, 0); // What should process A write into memory
    mem_int *i = shmat(shm_int, NULL, 0);
    int b_printed = 0;
    while(!(i->prog_b_done && i->prog_c_done)){
        if(i->prog_b_done && !b_printed){
            printf("PROGRAM B IS DONE\n");
            b_printed = 1;
        }
        else if(i->prog_c_done){
            printf("PROGRAM C IS DONE\n");
        }
    }

    printf("GOODBYE!");




}

