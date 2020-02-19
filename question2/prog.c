#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include "part1.h"

#define SHM_STR_KEY 4156
#define SHM_INT_KEY 5198

int main()
{
    printf("PROGRAM A RUNNING\n");
    int shm_str = shmget(SHM_STR_KEY, sizeof(mem_string), IPC_CREAT | 0777);
    int shm_int = shmget(SHM_INT_KEY, sizeof(mem_int), IPC_CREAT | 0777);

    mem_string *str = shmat(shm_str, NULL, 0);
    mem_int *i = shmat(shm_int, NULL, 0);

    int pidB = fork();
    int pidC = fork();
    int j;

    if (pidB == 0 && pidC > 0)
    {
        // Starting process B
        int shm_str = shmget(SHM_STR_KEY, sizeof(mem_string), 0);
        int shm_int = shmget(SHM_INT_KEY, sizeof(mem_int), 0);
        printf("B: WAITING FOR PROCESS A TO CREATE MEMORY\n");
        while (shm_str == -1 || shm_int == -1)
        {
            if (errno = EINVAL)
            {
                // Wait for memory to be created
            }
            else
            {
                perror("shmat");
                exit(-1);
            }
            shm_str = shmget(SHM_STR_KEY, sizeof(mem_string), 0);
            shm_int = shmget(SHM_INT_KEY, sizeof(mem_int), 0);
        }
        printf("B: MEMORY CREATED BY PROGRAM A\n");
        mem_string *str = shmat(shm_str, NULL, 0);
        mem_int *i = shmat(shm_int, NULL, 0);
        i->prog_b_done = 0;
        str->s = 's';
        str->h = 'h';
        str->a = 'a';
        str->r = 'r';
        str->e = 'e';
        str->d = 'd';

        i->prog_b_done = 1;
        printf("B: PROG B DONE\n");
        char *the_string = &str->s;

        printf("B: PROGRAM B IS COMPLETE. WROTE %s to memory.\n", the_string);

        while (i->prog_c_done != 1)
        {
        }

        printf("B: PROGRAM C IS DONE. wrote %s to memory.\n", the_string);

        exit(0);
    }

    
    else if(pidC == 0 && pidB > 0)
    {
        int shm_str = shmget(SHM_STR_KEY, sizeof(mem_string), 0);

        printf("C: WAITING FOR PROCESS A TO CREATE MEMORY\n");
        while (shm_str == -1 || shm_int == -1)
        {

            if (errno = EINVAL)
            {
                if (shm_int == -1)
                {
                    printf("C: INT MEMORY NOT CREATED. BRUH\n");
                }
                // Wait for memory to be created
            }
            else
            {
                perror("shmat");
                exit(-1);
            }
            shm_str = shmget(SHM_STR_KEY, sizeof(mem_string), 0);
            shm_int = shmget(SHM_INT_KEY, sizeof(mem_int), 0);
        }
        printf("C: MEMORY CREATED BY PROGRAM A\n");
        int shm_int = shmget(SHM_INT_KEY, sizeof(mem_int), 0);
        mem_string *str = shmat(shm_str, NULL, 0);
        mem_int *i = shmat(shm_int, NULL, 0);
        while (i->prog_b_done != 1)
        {
        }
        char *the_string = &str->s;

        printf("C: PROGRAM B IS COMPLETE. WROTE %s to memory.\n", the_string);
        i->prog_c_done = 0;
        str->s = 'm';
        str->h = 'e';
        str->a = 'm';
        str->r = 'o';
        str->e = 'r';
        str->d = 'y';
        printf("C: PROGRAM C IS DONE. wrote %s to memory.\n", the_string);
        i->prog_c_done = 1;

        exit(0);
    }
    else if(pidB > 0 && pidC > 0) {
        wait(NULL);
        printf("GOODBYE\n");
    }
}

