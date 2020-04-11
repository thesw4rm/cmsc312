#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>





#define BUF_SIZE 5
#define THREADS 6
#define LOOPS 2


int buf[BUF_SIZE];
int buf_ind;

pthread_mutex_t mut;
sem_t full;
sem_t empty;
void insert(int val){
    if(buf_ind < BUF_SIZE){
        buf[buf_ind++] = val;
    }
    else{
        printf("BUFFER OVERFLOW\n");
    }
}

int rm(){
    if(buf_ind > 0)
        return buf[--buf_ind];
    else
        printf("BUFFER UNDERFLOW\n");
    return 0;
}

void *prod(void *t_n){
    int t = *(int *)t_n;
    int val;
    int i = 0;
    while(i++ < LOOPS){
        sleep(rand() % 10);
        val = rand() % 100;
        sem_wait(&full);
        pthread_mutex_lock(&mut);
        insert(val);
        pthread_mutex_unlock(&mut);
        sem_post(&empty);
        printf("Prod %d added %d products to buffer\n", t, val);
    }
    pthread_exit(0);
}

void *cons(void *t_n){
    int t = *(int *)t_n;
    int val;
    int i = 0;
    while(i++ < LOOPS){
        sem_wait(&empty);
        pthread_mutex_lock(&mut);
        val = rm(val);

        pthread_mutex_unlock(&mut);
        sem_post(&full);
        printf("Cons %d removed %d products from buffer\n", t, val);
    }
    pthread_exit(0);
}

int main(){
    buf_ind = 0;
    pthread_mutex_init(&mut, NULL);
    sem_init(&full, // sem_t *sem
             0, // int pshared. 0 = shared between threads of process,  1 = shared between processes
             BUF_SIZE); // unsigned int value. Initial value
    sem_init(&empty,
             0,
             0);

    pthread_t thread[THREADS];
    int thread_numb[THREADS];
    int i;
    for (i = 0; i < THREADS;) {
        thread_numb[i] = i;
        pthread_create(thread + i, // pthread_t *t
                       NULL, // const pthread_attr_t *attr
                       prod, // void *(*start_routine) (void *)
                       thread_numb + i);  // void *arg
        i++;
        thread_numb[i] = i;
        // playing a bit with thread and thread_numb pointers...
        pthread_create(&thread[i], // pthread_t *t
                       NULL, // const pthread_attr_t *attr
                       cons, // void *(*start_routine) (void *)
                       &thread_numb[i]);  // void *arg
        i++;


    }

    for (i = 0; i < THREADS; i++)
        pthread_join(thread[i], NULL);

    pthread_mutex_destroy(&mut);
    sem_destroy(&full);
    sem_destroy(&empty);

    return 0;
}
