#include "constants.h"
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>


shm_mdata *mdata;
job *pq;

volatile int sigint_tripped; // Becomes 1 if signal interrupt is received.
pid_t *uprocs;
pthread_t *ptds;
int num_uprocs;
void main_sigint_handler(int sig_num) {
    for (int i = 0; i < num_uprocs; i++) {
        kill(uprocs[i], SIGINT);
    }
    for(int i = 0; i < num_uprocs; i++){

        int status;
        waitpid(uprocs[i], &status, 0);
    }
}
void sigint_handler(int sig_num) { sigint_tripped = 1; }
void print_bits(unsigned int n) {
    unsigned int mask = 1 << 29;
    while (mask != 0) {
        printf("%u", (n & mask) / mask);
        mask >>= 1;
    }
    printf("\n");
}

void write_data_to_file(FILE *csv_f, int is_printer, int printer_id,
                        int user_id, int bytes, long int start_time,
                        long int wait_time) {

    fprintf(csv_f, "%d,%d,%d,%d,%lu,%lu\n", is_printer, printer_id, user_id,
            bytes, start_time, wait_time);
}

unsigned int get_next_free_off(unsigned int free_space) {
    unsigned int noff = 0;
    while ((free_space & (1 << noff)) == 0) {
        ++noff;
    }
    // print_bits(free_space);
    return noff;
}

unsigned int set_off_not_free(unsigned int free_space, unsigned int off) {
    unsigned int mask = 0x3fffffff ^ (1 << off); // Mask is for 30 bits
    return free_space & mask;
}

unsigned int set_off_free(unsigned int free_space, unsigned int off) {
    unsigned int mask = 1 << off;
    return free_space | mask;
}

void user(int tn, int seed) { //(void *thread_n) {
    signal(SIGINT, sigint_handler);
    srand(seed);
    
    int num_jobs = rand() % 30 + 1;
    printf("User %d is creating %d jobs\n", tn, num_jobs);
    int i;
    for (i = 0; i < num_jobs; i++) {
        usleep(rand() % 900000 + 100000);

        csema_wait(&mdata->qfull);
        int bytes = rand() % 900 + 100;
        unsigned int noff = 0;
        int free_found = 0;
        while (free_found == 0) {
            if (sem_trywait(&mdata->pqmut[noff]) >= 0) {
                // Try to wait, if it is being used then move on to next slot
                if (pq[noff].f == 0) {
                    // Not free, must keep going
                    sem_post(&mdata->pqmut[noff]);
                    ++noff;
                } else {
                    // We found a free location
                    free_found = 1;
                }
            }
        }
        job *njob = &pq[noff];
        njob->f = 0; // Set as not free
        init_fcfs_job(njob, bytes, noff);

        njob->start_wait_time = clock();
        njob->thd = tn;
        sem_wait(&mdata->qmut);
        if (mdata->head != NULL) {
            add_job_fcfs(mdata->head, njob);
        } else {
            mdata->head = njob;
        }
        sem_post(&mdata->qmut);

        sem_post(&mdata->pqmut[noff]); // Unlock the slot for use by printer
        csema_post(&mdata->qempty);
        printf("User %d created job of %d bytes\n", tn, bytes);
        if(sigint_tripped == 1){
            printf("User %d caught SIGINT after completing %d jobs\n", tn, i+1);
            break;
        }
    }
    printf("USER PROC %u IS FINISHED\n", tn);
    sem_wait(&mdata->qmut);
    mdata->user_procs_left--;
    if (mdata->user_procs_left == 0) {
        csema_post(&mdata->uprocs_comp);
    }
    sem_post(&mdata->qmut);
    exit(0);
}

void print_handler(void *t_n) { printf("Printer %d cancelled\n", *(int *)t_n); }

/*compl_job->start_wait_time,
        (cur_time - compl_job->start_wait_time)*/

void *print(void *thread_n) {
    int tn = *(int *)thread_n; // Thread number
    printf("Printer %d started %d\n", tn, mdata->user_procs_left);
    job *compl_job; // Store completed job

    while (1) {

        csema_wait(&mdata->qempty);
        sem_wait(&mdata->qmut);
        if (mdata->head == NULL && mdata->user_procs_left == 0) {
            csema_post(&mdata->pprocs_comp);
        }

        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        // TODO: Subtract the overhead time of this stuff
        compl_job = mdata->head;

        long wait = compl_job->bytes * 5 * 1000;
        unsigned int bytes = compl_job->bytes;
        unsigned int thd = compl_job->thd;
        mdata->head = rm_job(compl_job);
        compl_job->f = 1; // Set job as free because we are printing it
        if (mdata->head == NULL && mdata->user_procs_left == 0) {
            csema_post(&mdata->pprocs_comp);
        }
        sem_post(&mdata->qmut);

        csema_post(&mdata->qfull);
        usleep(wait);
        clock_t cur_time;
        cur_time = clock();
        printf("Printer %d printed job of %d bytes created by user %d\n", tn,
               bytes, thd);

        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    }
}

int main(int argc, char **argv) {
    signal(SIGINT, main_sigint_handler);
    int user_procs, print_procs;
    if (argc < 3) {
        user_procs = DEFAULT_UPROCS;
        print_procs = DEFAULT_PPROCS;
    } else {
        user_procs = (int)strtol(argv[1], (char **)NULL, 10);
        print_procs = (int)strtol(argv[2], (char **)NULL, 10);
    }
    size_t tot_size = 0;
    mdata = mmap(NULL, DEFAULT_SHM_SIZE, PROT_READ | PROT_WRITE,
                 MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    pq = mmap(NULL, DEFAULT_PQUEUE_SIZE, PROT_READ | PROT_WRITE,
              MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    mdata->user_procs_left = user_procs;
    csema_init(&mdata->qempty, 0);
    csema_init(&mdata->qfull, PRINT_QUEUE_LIMIT);
    csema_init(&mdata->uprocs_comp, 0);
    csema_init(&mdata->pprocs_comp, 0);

    sem_init(&mdata->qmut, 1, 1);
    for (int i = 0; i < 30; i++)
        sem_init(&mdata->pqmut[i], 1, 1);
    for (int i = 0; i < 30; i++)
        pq[i].f = 1; // Set all jobs as free initially
    mdata->head = NULL;
    // Do this in case queue size is variable (change qmut to pointer)
    // mdata->qmut = mmap(NULL, sizeof(sem_t) * 30, PROT_READ|PROT_WRITE,
    // MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    uprocs = malloc(sizeof(pid_t) * user_procs);
    ptds = malloc(sizeof(pthread_t) * print_procs);
    int ptd_num[print_procs];
    int utd_num[user_procs];
    int i = 0;

    srand(clock());
    while (i < user_procs || i < print_procs) {
        pid_t pid;
        if (i < user_procs) {
            pid = fork();
            int seed = rand();
            if (pid == 0) {
                free(uprocs);
                free(ptds);
                user(i, seed);
                exit(0);
            }
            else {
                uprocs[i] = pid;
            }
        }

        if (i < print_procs) {
            ptd_num[i] = i;
            pthread_create(&ptds[i], NULL, print, &ptd_num[i]);
        }
        i++;
    }

    csema_wait(&mdata->uprocs_comp);
    printf("USER PROCS DONE\n");
    csema_wait(&mdata->pprocs_comp);
    for (i = 0; i < print_procs; i++) {

        pthread_cancel(ptds[i]);
    }

    for (i = 0; i < print_procs; i++) {
        pthread_join(ptds[i], NULL);
    }
    sem_destroy(&mdata->qmut);
    csema_destroy(&mdata->qempty);
    csema_destroy(&mdata->qfull);
    csema_destroy(&mdata->uprocs_comp);
    csema_destroy(&mdata->pprocs_comp);
    for (int i = 0; i < 30; i++)
        sem_destroy(&mdata->pqmut[i]);
    munmap(mdata, DEFAULT_SHM_SIZE);
    munmap(pq, DEFAULT_PQUEUE_SIZE);
    free(uprocs);
    free(ptds);
}