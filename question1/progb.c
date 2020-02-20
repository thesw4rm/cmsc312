#include <stdio.h>
#include <stdint.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <errno.h>
#include <stdlib.h>

#include "part1.h"


#define SHM_STR_KEY 2113
#define SHM_INT_KEY 2114

int main(){
    

     int shm_str = shmget(SHM_STR_KEY, sizeof(mem_string), 0);
    
    printf("WAITING FOR PROCESS A TO CREATE MEMORY\n");
    while(shm_str == -1){
        if(errno == ENOENT){
            // Wait for memory to be created
        }
        else{
            perror("shmget");
            exit(-1);
        }
        shm_str = shmget(SHM_STR_KEY, sizeof(mem_string), 0);
    }
    printf("MEMORY CREATED BY PROGRAM A\n");
    int shm_int = shmget(SHM_INT_KEY, sizeof(mem_int), 0);
    mem_string *str = shmat(shm_str, NULL, 0);
    mem_int *i = shmat(shm_int, NULL, 0);
    printf("SHMAT DONE\n");
    i->prog_b_done = 0;
    str->s = 's';
    str->h = 'h';
    str->a = 'a';
    str->r = 'r';
    str->e = 'e';
    str->d = 'd';
    
    i->prog_b_done = 1;
    printf("PROG B DONE\n");
    volatile char *the_string = &str->s;


    printf("PROGRAM B IS COMPLETE. WROTE %s to memory.\n", the_string);

    while(i->prog_c_done != 1){

    }
    
    printf("PROGRAM C IS DONE. wrote %s to memory.\n", the_string);
    shmdt(&str);
    shmdt(&i);

}

