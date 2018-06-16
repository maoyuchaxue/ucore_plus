#ifndef BARRIERS_H
#define BARRIERS_H
#include <ulib.h>

typedef struct {
    volatile int needed;
    volatile int called;
    sem_t mutex;
    sem_t cond[20] ;
    //pthread_cond_t cond;
} barrier_t;


int barrier_init(barrier_t *barrier,int needed);
int barrier_destroy(barrier_t *barrier);
int barrier_wait(barrier_t *barrier);

#endif

