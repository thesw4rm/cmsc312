#include <stdio.h>
#include <stdint.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <errno.h>
#include <stdlib.h>
#include "part1.h"


#define SHM_STR_KEY 4156
#define SHM_INT_KEY 4157

int main(){
 
    int shm_str = shmget(SHM_STR_KEY, sizeof(mem_string), 0);
    
    printf("WAITING FOR PROCESS A TO CREATE MEMORY\n");
    while(shm_str == -1){
        if(errno = EINVAL){
            // Wait for memory to be created
        }
        else{
            perror("shmat");
            exit(-1);
        }
        shm_str = shmget(SHM_STR_KEY, sizeof(mem_string), 0);
    }
    printf("MEMORY CREATED BY PROGRAM A\n");
    int shm_int = shmget(SHM_INT_KEY, sizeof(mem_int), 0);
    mem_string *str = shmat(shm_str, NULL, 0);
    mem_int *i = shmat(shm_int, NULL, 0);
    while(i->prog_b_done != 1){

    }
    volatile char *the_string = &str->s;


    printf("PROGRAM B IS COMPLETE. WROTE %s to memory.\n", the_string);
    i->prog_c_done = 0;
    str->s = 'm';
    str->h = 'e';
    str->a = 'm';
    str->r = 'o';
    str->e = 'r';
    str->d = 'y';
    printf("PROGRAM C IS DONE. wrote %s to memory.\n", the_string);
    i->prog_c_done = 1;
    shmdt(&str);
    shmdt(&i);

   




}

