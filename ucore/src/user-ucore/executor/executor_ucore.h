// Copyright 2017 syzkaller project authors. All rights reserved.
// Use of this source code is governed by Apache 2 LICENSE that can be found in the LICENSE file.

#ifndef _EXECUTOR_UCORE
#define _EXECUTOR_UCORE

//#include <pthread.h>
#include <atomic.h>

//typedef pthread_t osthread_t;

/*void thread_start(osthread_t* t, void* (*fn)(void*), void* arg)
{
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 128 << 10);
	if (pthread_create(t, &attr, fn, arg))
		exitf("pthread_create failed");
	pthread_attr_destroy(&attr);
}*/

struct event_t {
	atomic_t state;
};

void event_init(struct event_t* ev)
{
	atomic_set(&ev->state, 0) ;
}

void event_reset(struct event_t* ev)
{
	atomic_set(&ev->state, 0) ;
}

void event_set(struct event_t* ev)
{
	if (ev->state.counter)
		fail("event already set");
	atomic_set(&ev->state, 1) ;
	//syscall(SYS_futex, &ev->state, FUTEX_WAKE);
}

/*void event_wait(event_t* ev)
{
	while (!atomic_read(&ev->state))
		syscall(SYS_futex, &ev->state, FUTEX_WAIT, 0, 0);
}*/

bool event_isset(struct event_t* ev)
{
	return atomic_read(&ev->state);
}

/*
bool event_timedwait(event_t* ev, uint64 timeout_ms)
{
	uint64 start = current_time_ms();
	uint64 now = start;
	for (;;) {
		timespec ts = {};
		ts.tv_sec = 0;
		ts.tv_nsec = (timeout_ms - (now - start)) * 1000 * 1000;
		syscall(SYS_futex, &ev->state, FUTEX_WAIT, 0, &ts);
		if (atomic_read(&ev->state))
			return true;
		now = current_time_ms();
		if (now - start > timeout_ms)
			return false;
	}
}
*/

#endif
