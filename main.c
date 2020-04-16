
#include <sys/types.h>
#include <sys/wait.h>

#include "constants.h"
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <unistd.h>

shm_mdata *mdata;
job *pq;

volatile int sigint_tripped; // Becomes 1 if signal interrupt is received.
pid_t *uprocs;
pthread_t *ptds;
int num_uprocs;
void main_sigint_handler(int sig_num) {
    for (int i = 0; i < num_uprocs; i++) {
        kill(uprocs[i], SIGINT);
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

void user(int tn, int seed) {
    srand(seed);
    signal(SIGINT, sigint_handler);
    int shm_fd = shmget(SHM_KEY, DEFAULT_SHM_SIZE, IPC_CREAT | 0777);
    int pqueue_fd = shmget(PQUEUE_SIZE, DEFAULT_PQUEUE_SIZE, IPC_CREAT | 0777);
    shm_mdata *md = shmat(shm_fd, NULL, 0);
    job *pqueue = shmat(pqueue_fd, NULL, 0);

    int num_jobs = rand() % 30 + 1;
    printf("User %d is creating %d jobs\n", tn, num_jobs);
    int i;
    for (i = 0; i < num_jobs; i++) {
        usleep(rand() % 900000 + 100000);

        csema_wait(&md->qfull);
        pthread_mutex_lock(&md->qmut);
        int bytes = rand() % 900 + 100;
        unsigned int noff = get_next_free_off(md->free_space);
        job *njob = &pqueue[noff];
        init_sjf_job(njob, bytes, noff);

        njob->start_wait_time = clock();
        njob->thd = tn;
        njob->off = noff;
        if (md->head >= 0) {
            md->head = add_job_sjf(&pqueue[md->head], njob);
        } else {
            md->head = njob->off;
        }

        md->free_space = set_off_not_free(md->free_space, noff);

        pthread_mutex_unlock(&md->qmut);
        csema_post(&md->qempty);

        printf("User %d created job of %d bytes\n", tn, bytes);
        if (sigint_tripped == 1) {

            printf("USER %d received SIGINT. Stopping execution at %d jobs\n",
                   tn, i + 1);
            break;
        }
    }
    printf("USER PROC %u IS FINISHED\n", tn);
    pthread_mutex_lock(&md->qmut);
    md->user_procs_left--;
    if (md->user_procs_left == 0) {
        csema_post(&md->uprocs_comp);
    }
    free(ptds);
    free(uprocs);
    pthread_mutex_unlock(&md->qmut);
    exit(0);
}

void print_handler(void *t_n) { printf("Printer %d cancelled\n", *(int *)t_n); }

void *print(void *thread_n) {
    int tn = *(int *)thread_n; // Thread number
    int shm_fd = shmget(SHM_KEY, DEFAULT_SHM_SIZE, IPC_CREAT | 0777);
    int pqueue_fd = shmget(PQUEUE_SIZE, DEFAULT_PQUEUE_SIZE, IPC_CREAT | 0777);
    shm_mdata *md = shmat(shm_fd, NULL, 0);
    job *pqueue = shmat(pqueue_fd, NULL, 0);
    printf("Printer %d started %d\n", tn, md->user_procs_left);
    job *compl_job; // Store completed job

    while (1) {

        csema_wait(&md->qempty);
        /*if (!(mdata->user_procs_left > 0 || mdata->qhead != NULL))
            break;*/
        pthread_mutex_lock(&md->qmut);
        if (md->head == -1 && md->user_procs_left == 0) {
            csema_post(&md->pprocs_comp);
        }

        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        compl_job = &pqueue[md->head];

        long wait = compl_job->bytes * 5 * 1000;
        unsigned int bytes = compl_job->bytes;
        unsigned int thd = compl_job->thd;
        unsigned int off = compl_job->off;
        md->head = rm_job(compl_job);
        mdata->free_space = set_off_free(mdata->free_space, off);
        if (md->head == -1 && md->user_procs_left == 0) {
            csema_post(&md->pprocs_comp);
        }
        pthread_mutex_unlock(&md->qmut);

        csema_post(&md->qfull);
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
    num_uprocs = user_procs;
    size_t tot_size = 0;
    int shm_fd = shmget(SHM_KEY, DEFAULT_SHM_SIZE, IPC_CREAT | 0777);
    int pqueue_fd = shmget(PQUEUE_SIZE, DEFAULT_PQUEUE_SIZE, IPC_CREAT | 0777);
    mdata = shmat(shm_fd, NULL, 0);
    pq = shmat(pqueue_fd, NULL, 0);
    mdata->user_procs_left = user_procs;
    csema_init(&mdata->qempty, 0);
    csema_init(&mdata->qfull, PRINT_QUEUE_LIMIT);
    csema_init(&mdata->uprocs_comp, 0);
    csema_init(&mdata->pprocs_comp, 0);
    mdata->free_space = 0x3fffffff; // Set all bits to 1
    mdata->head = -1;
    pthread_mutex_init(&mdata->qmut, NULL);
    ptds = malloc(sizeof(pthread_t) * print_procs);
    uprocs = malloc(sizeof(pid_t) * user_procs);
    //    pthread_t utds[user_procs];
    int ptd_num[print_procs];
    int utd_num[user_procs];
    int i = 0;
    srand(clock());
    while (i < user_procs || i < print_procs) {
        pid_t pid;
        if (i < user_procs) {
            //              utd_num[i] = i;
            //            pthread_create(&utds[i], NULL, user, &utd_num[i]);
            pid = fork();
            int seed = rand();
            if (pid == 0) {
                user(i, seed);
                exit(0);
            } else {
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
    // printf("UPROCS DONE\n");
    csema_wait(&mdata->pprocs_comp);
    /* *
     * Send cancel command to each thread before joining, so one thread that takes a long time doesn't block the other ones
     * */
    for (i = 0; i < print_procs; i++) {
        pthread_cancel(ptds[i]);
    }
    /* *
     * Wait for each thread to stop
     * */
    for (i = 0; i < print_procs; i++) {
        pthread_join(ptds[i], NULL);
    }
    pthread_mutex_destroy(&mdata->qmut);
    csema_destroy(&mdata->qempty);
    csema_destroy(&mdata->qfull);
    csema_destroy(&mdata->uprocs_comp);
    csema_destroy(&mdata->pprocs_comp);
    shmdt(mdata);
    if (shmctl(shm_fd, IPC_RMID, NULL) < 0)
        perror("shmctl");
    if (shmctl(pqueue_fd, IPC_RMID, NULL) < 0)
        perror("shmctl");
    free(uprocs);
    free(ptds);
}
