
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "ski-hyper.h"
#include "ski-barriers.h"
#include <syscall.h>
#include <ulib.h>
#include <stat.h>

#define MAX_SAFE_STACK 2*1024
#define MAX_CPUS 16


//int with_hypercall = 0; Replaced by STD_SKI_HYPERCALLS
extern int STD_SKI_HYPERCALLS;
extern int STD_SKI_SOFT_EXIT_BARRIER;

//int with_soft_exit_barrier = 0;
int with_barriers = 1;
int with_info = 1;
int with_stack_prefault = 1;
int with_mlock_prefault = 0;
int with_fork = 1;
//int mlock_parametrs = MCL_CURRENT| MCL_FUTURE;
int max_instructions = 1;  // Can be overwritten at the command line

//pthread_mutex_t * volatile printf_mutex = NULL;
barrier_t* usermode_barrier ;
sem_t* printf_sem = NULL;

char buf[1000] ;

#define PROT_READ 0x1 
#define PROT_WRITE 0x2 
#define MAP_SHARED 0x1
#define MAP_PRIVATE 0x2
#define MAP_FIXED 0x10
#define MAP_ANON 0x20

int buildFile(int size, char* name)
{
    int fd = open(name, O_RDWR | O_CREAT);// shm_open只是一个更加简便、高效的利用tmpfs的方法，这里直接替换成open
    cprintf("shared fd: %d\n", fd) ;

    struct stat stat ;
    fstat(fd, &stat) ;
    cprintf("detected file length: %d\n", stat.st_size) ;
    if(stat.st_size < 2) // empty_file
        write(fd, buf, 500) ; // fill the file 
    return fd ;
}

static void* shared_malloc(int size, char* name) { // checked
    //int fd = buildFile(size, name) ;
    void* ptr = sys_linux_mmap(0,size,
        PROT_READ+PROT_WRITE,
        0x1 | MAP_FIXED | MAP_ANON, // MAP_SHARED
        -1,0) ;
    cprintf("map finished\n") ;
    cprintf("ptr: %lx\n", ptr) ;
    return ptr ;
}

void shared_printf_init(int current_cpu){ // checked
	
	static int is_init = 0;
	
	if (is_init){
		return;
	}
	is_init = 1;

    if(with_fork){
        printf_sem = shared_malloc(sizeof(sem_t), "SKI_shared_printf");
        if(current_cpu==0){
            *printf_sem = sem_init(1);
			/*pthread_mutexattr_init(&attr);
		    if(pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED)){
			    perror("Mutex PTHREAD_PROCESS_SHARED");
				exit(1);
	        }
	
		    pthread_mutex_init(printf_mutex, &attr);*/
		}
    }
    cprintf("printf init finished\n") ;
}

void shared_printf(char *format,...) // checked
{
    va_list args;
    va_start(args, format);

    if(with_fork){
        sem_wait(printf_sem) ;
		cprintf("[SKI] ");
        vcprintf(format, args);
        //fflush(stdout);
        sem_post(printf_sem) ;
    }else{
		cprintf("[SKI] ");
        vcprintf(format, args);
    }
    va_end(args);
    return;
}


static void stack_prefault(void) { // checked
    unsigned char dummy[MAX_SAFE_STACK];

    memset(dummy, 0, MAX_SAFE_STACK);
    return;
}

hypercall_io hio_enter;
hypercall_io hio_exit;

//actually it's process_start
static int ski_thread_start(int thread_num, int total_cpus, int dry_run){ // checked
    char str[256];
    int pid;

    memset(str, 0, sizeof(str));

    if(with_stack_prefault){
        stack_prefault();
    }

	if(dry_run){
		return 0;
	}

    // deleted since uCore don't have gettid syscall.(although it has tid)
    pid = getpid();
    shared_printf("[%d] Process id: %d\n", thread_num, pid);

    // setAffinity, uCore don't have this
    /*    
    pthread_attr_t attr;
    cpu_set_t cpuset;
    pthread_t self;
    self = pthread_self();
    pthread_getattr_np(self, &attr);
    int ret = pthread_attr_getaffinity_np(&attr, sizeof(cpuset), &cpuset);
    if (ret){
        shared_printf("[%d] Error: Unable get thread attributes!\n", thread_num);
        exit(1);
    }*/

    // initialize the hypercall parameters
    hypercall_io_init(&hio_enter);
    hypercall_io_init(&hio_exit);
    hio_enter.hypercall_type = HYPERCALL_IO_TYPE_TEST_ENTER;
    hio_exit.hypercall_type = HYPERCALL_IO_TYPE_TEST_EXIT;
    hio_enter.p.hio_test_enter.gh_nr_instr = max_instructions;
    hio_enter.p.hio_test_enter.gh_nr_cpus = total_cpus;
    hio_enter.p.hio_test_enter.gh_disable_interrupts = 1;

    if(with_barriers){
		shared_printf("[%d] Going into usermode barrier\n", thread_num );
        barrier_wait(usermode_barrier);
		shared_printf("[%d] Passed the usermode barrier\n", thread_num ); //XXX: Should take this out...
    }else{
		shared_printf("[%d] Usermode barrier disabled\n", thread_num );
	}
	
	int hypercall_ret = 0;

    if(STD_SKI_HYPERCALLS){
        hypercall(&hio_enter);
		hypercall_ret = hio_enter.p.hio_test_enter.hg_res;
    }
	return hypercall_ret;
	// Actual test run just after this
}

static void ski_thread_finish(int thread_num, int dry_run){ // checked
	// Actual test ran just before this

	if(dry_run){
		// Mostly meant to just pre fault the data with the function code
		return;
	}

	if(STD_SKI_SOFT_EXIT_BARRIER){
		barrier_wait(usermode_barrier);
	}


    if(STD_SKI_HYPERCALLS){
        hypercall(&hio_exit);
        shared_printf("[%d] Finished (hypercall enabled)\n", thread_num );
    }else{
        shared_printf("[%d] Finished (hypercall disabled)\n", thread_num );
    }

    if(with_barriers){
        barrier_wait(usermode_barrier);
    }
}

 
void ski_tests_init(int current_cpu, int total_cpus){ // checked
    shared_printf_init(current_cpu);

    if(with_barriers || STD_SKI_SOFT_EXIT_BARRIER){
        usermode_barrier = shared_malloc(sizeof(barrier_t), "SKI_barrier");
        if(current_cpu == 0){
			barrier_init(usermode_barrier, total_cpus);
		}
		cprintf("Sleeping for 5 seconds.\n");
		sleep(5);
    }
}

int ski_test_start(int current_cpu, int total_cpus, int dry_run){ // checked
	//cpu_set_t  cpuset_process;
	int ret;

	if(dry_run){
		ret  = ski_thread_start(current_cpu, total_cpus, dry_run);
		return ret;
	}

/*    shared_printf_init(current_cpu);
*/

/*
    if(with_barriers){
        usermode_barrier = shared_malloc(sizeof(barrier_t), "SKI_barrier", current_cpu);
        if(current_cpu == 0){
			barrier_init(usermode_barrier, total_cpus);
		}
    }
*/
    /*if(with_mlock_prefault){
        if(mlockall(mlock_parametrs) == -1) {
            perror("SKI: mlockall failed");
            exit(-2);
        }
    }*/

/*	XXX: What was this for???
    if(signal(SIGINT, sig_handler)== SIG_ERR){
        perror("SKI: Signal SIGINT error");
        exit(1);
    }
    if(signal(SIGSEGV , sig_handler) == SIG_ERR){
        perror("SKI: Signal SIGINT error");
        exit(1);
    }
*/

	// Avoid having unflushed disk before the test begins...
	// XXX: but this could potentially interfere with tests
    //fflush(0);
    //sync();

	//shared_printf("We are process %d\n", current_cpu);

	/* uCore has no affinity mask so this part is deleted.
    CPU_ZERO(&cpuset_process);
	CPU_SET(current_cpu, &cpuset_process);
	ret = sched_setaffinity(0,sizeof(cpuset_process),&cpuset_process);
	if (ret) {
		shared_printf("SKI: Error: Failed to set up the affinity for thread 3!\n");
		perror("SKI: sched_setaffinity");
		exit(1);
	}**/

	ret = ski_thread_start(current_cpu, total_cpus, dry_run);
	return ret;

	// Test runs afterwards
}


void ski_test_finish(int current_cpu, int dry_run){ // checked
	
	if(dry_run){
		ski_thread_finish(current_cpu, dry_run);
		return;
	}

	// Test ran before
	ski_thread_finish(current_cpu, dry_run);

	// Printf: waiting on the barrier
	// Should have a barrier here for all the tests
	// Printf: finished the test

    if(with_barriers || STD_SKI_SOFT_EXIT_BARRIER){
        //barrier_destroy(usermode_barrier);
    }


}


