#include <stdio.h>
#include <stdlib.h>

#include "ski-barriers.h"

extern int STD_SKI_ENABLED, STD_SKI_WAIT_FOR_RESULTS, STD_SKI_CPU_AFFINITY, STD_SKI_HYPERCALLS, STD_SKI_SOFT_EXIT_BARRIER, STD_SKI_USER_BARRIER, STD_SKI_TOTAL_CPUS, STD_SKI_TEST_NUMBER, STD_SKI_PROFILE_ENABLED;
extern int SKI_TEST_COUNTER;
int current_cpu ;
int ski_test_start(int current_cpu, int total_cpus, int dry_run);
void ski_test_finish(int current_cpu, int dry_run);
void hypercall_debug(int current_cpu, char *format, ...);
void hypercall_debug_quiet(int current_cpu, char *format, ...);
void * shared_malloc(int size, char* name, int current_cpu) ;

void preWork() // need update 
{
	buildFile(sizeof(sem_t), "SKI_shared_printf") ;// dry run, in order to build the file
	buildFile(sizeof(barrier_t), "SKI_barrier") ;// dry run, in order to build the file
	// the current_cpu is actually a fake param, 
	// cause it's only used for debugging, and since uCore doen't 
	// have the ability to keep a process running in a single CPU, this info becomes unuseful at all. 
	int i ;
	for(int i = 0; i < STD_SKI_TOTAL_CPUS-1; i ++)
	{
		current_cpu = i+1 ;
		int pid = fork() ;
		if(pid == 0) break ;
	}
	if(i == STD_SKI_TOTAL_CPUS-1) current_cpu = 0 ;

	// need update to shutdown the swap operations of memory in uCore
		//setrlimit(RLIMIT_MEMLOCK, &rlim);
		// int mlock_parametrs = MCL_CURRENT; //| MCL_FUTURE;
		// if(mlockall(mlock_parametrs) == -1) {
		// 	perror("mlockall failed");
		// 	exit(-2);
		// }

	//exec >  >(xargs -d '\n' -n 1 ${DEBUG_BINARY})
	//exec 2> >(xargs -d '\n' -n 1 ${DEBUG_BINARY} >&2)

	// Update the affinity of the internal kernel threads
	//./misc/update-affinity.sh
	
}

int main(){
	
	cprintf("Start prework\n");
	preWork();
	cprintf("Empty test\n");

	ski_parse_env();
	cprintf("test init finished.\n") ;

	// To avoid page fault
	//hypercall_debug(STD_SKI_CPU_AFFINITY, (char*)"About to start test [TEST] - CPU: %d Op: %s Op_seed: %d Op_no: %d Op_max: %d Test_seed: %d ",
 	//			STD_SKI_CPU_AFFINITY, "empty", -1, -1, 0, test_seed);


	hypercall_debug_quiet(STD_SKI_CPU_AFFINITY, (char*)"First call");


    ski_test_start(STD_SKI_CPU_AFFINITY, STD_SKI_TOTAL_CPUS, 1);


    // Actual run
    int ret = ski_test_start(STD_SKI_CPU_AFFINITY, STD_SKI_TOTAL_CPUS, 0);
    int test_seed = ret + STD_SKI_CPU_AFFINITY;

	hypercall_debug_quiet(STD_SKI_CPU_AFFINITY, (char*)"Start test [TEST] - CPU: %d Op: %s Op_seed: %d Op_no: %d Op_max: %d Test_seed: %d ",
			STD_SKI_CPU_AFFINITY, "empty", -1, -1, 0, test_seed);


    ski_test_finish(STD_SKI_CPU_AFFINITY,0);
	if (STD_SKI_CPU_AFFINITY==0){
		hypercall_debug_quiet(STD_SKI_CPU_AFFINITY, (char*)"END INFO");
	}


	hypercall_debug_quiet(STD_SKI_CPU_AFFINITY, "Guest finished snapshot") ;

	return 0;
}

