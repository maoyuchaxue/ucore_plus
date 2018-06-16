#include <stdlib.h>
#include <stdio.h>

#include "ski-barriers.h"

// From http://www.howforge.com/implementing-barrier-in-pthreads

extern int with_fork;

int barrier_init(barrier_t *barrier,int needed)
{
    barrier->needed = needed;
    barrier->called = 0;

	barrier->mutex = sem_init(1) ; 
	for(int i = 0; i < needed; i ++)
		barrier->cond[i] = sem_init(1) ; 
    return 0;
}


int barrier_destroy(barrier_t *barrier)
{
    return 0;
}


int barrier_wait(barrier_t *barrier)
{
	sem_wait(barrier->mutex) ;
    barrier->called++;
    if (barrier->called == barrier->needed) {
        barrier->called = 0;
		for(int i = 0; i < barrier->needed; i ++) // like broadcast
			sem_post(barrier->cond[i]) ;
		sem_post(barrier->mutex) ;
    } else {
		sem_post(barrier->mutex) ;
		sem_wait(barrier->cond[barrier->called-1]) ;
		sem_wait(barrier->mutex) ;
    }
	sem_post(barrier->mutex) ;
    return 0;
}

