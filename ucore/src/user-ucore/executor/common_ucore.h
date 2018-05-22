// Copyright 2016 syzkaller project authors. All rights reserved.
// Use of this source code is governed by Apache 2 LICENSE that can be found in the LICENSE file.

// This file is shared between executor and csource package.

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

// *** Since SYZ_EXECUTOR is defined, all librarys will be defined 
// *** Here is the sum up of those .h files:
#include <ulib.h>
#include <atomic.h>
#include <syscall.h>
#include <unistd.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <mount.h>
#include <stat.h>
#include <types.h>
#include <string.h>
#include <dir.h>
#include <resource.h>

#if defined(SYZ_EXECUTOR) || (defined(SYZ_REPEAT) && defined(SYZ_WAIT_REPEAT)) ||      \
    defined(SYZ_USE_TMP_DIR) || defined(SYZ_HANDLE_SEGV) || defined(SYZ_TUN_ENABLE) || \
    defined(SYZ_SANDBOX_NAMESPACE) || defined(SYZ_SANDBOX_SETUID) ||                   \
    defined(SYZ_SANDBOX_NONE) || defined(SYZ_FAULT_INJECTION) ||                       \
    defined(__NR_syz_kvm_setup_cpu) || defined(__NR_syz_init_net_socket) && (defined(SYZ_SANDBOX_NONE) || defined(SYZ_SANDBOX_SETUID) || defined(SYZ_SANDBOX_NAMESPACE))
// One does not simply exit.
// _exit can in fact fail.
// syzkaller did manage to generate a seccomp filter that prohibits exit_group syscall.
// Previously, we get into infinite recursion via segv_handler in such case
// and corrupted output_data, which does matter in our case since it is shared
// with fuzzer process. Loop infinitely instead. Parent will kill us.
// But one does not simply loop either. Compilers are sure that _exit never returns,
// so they remove all code after _exit as dead. Call _exit via volatile indirection.
// And this does not work as well. _exit has own handling of failing exit_group
// in the form of HLT instruction, it will divert control flow from our loop.
// So call the syscall directly.
__attribute__((noreturn)) static void doexit(int status) // checked
{
	volatile unsigned i;
	syscall(SYS_exit, status);
	for (i = 0;; i++) {
	}
}
#endif

#include "common.h"

#if defined(SYZ_EXECUTOR)
//struct thread_t;
//void cover_reset(thread_t* th);
#endif

#if defined(SYZ_EXECUTOR) || defined(SYZ_HANDLE_SEGV)
#define _JBLEN  10      /* size, in longs, of a jmp_buf */
typedef long jmp_buf[_JBLEN]; 
static atomic_t skip_segv;
static jmp_buf segv_env;

//以下的三个函数主要是为了考虑到在进行生成的程序中可能存在一些违法地址而导致SIGSEGV，解决办法便是通过sigaction调用segv_handler函数，
//但是为了简化先不考虑这个
/*static void segv_handler(int sig, siginfo_t* info, void* uctx) // need update
{
	
	// Generated programs can contain bad (unmapped/protected) addresses,
	// which cause SIGSEGVs during copyin/copyout.
	// This handler ignores such crashes to allow the program to proceed.
	// We additionally opportunistically check that the faulty address
	// is not within executable data region, because such accesses can corrupt
	// output region and then fuzzer will fail on corrupted data.
	uintptr_t addr = (uintptr_t)info->si_addr;
	const uintptr_t prog_start = 1 << 20;
	const uintptr_t prog_end = 100 << 20;
	if (atomic_read(&skip_segv) && (addr < prog_start || addr > prog_end)) {
		debug("SIGSEGV on %p, skipping\n", (void*)addr);
		_longjmp(segv_env, 1);
	}
	debug("SIGSEGV on %p, exiting\n", (void*)addr);
	doexit(sig);
}*/

static void install_segv_handler() // checked
{
	/*
	struct sigaction sa;

	// Don't need that SIGCANCEL/SIGSETXID glibc stuff.
	// SIGCANCEL sent to main thread causes it to exit
	// without bringing down the whole group.
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sigaction(0x20, &sa, NULL) ;
	sigaction(0x21, &sa, NULL) ;
	// 这个8是什么意义呢，还不明
	//syscall(SYS_rt_sigaction, 0x20, &sa, NULL, 8);
	//syscall(SYS_rt_sigaction, 0x21, &sa, NULL, 8);

	memset(&sa, 0, sizeof(sa));
	sa.sa_sigaction = segv_handler;
	sa.sa_flags = SA_NODEFER | SA_SIGINFO;
	sigaction(SIGSEGV, &sa, NULL); 
	sigaction(SIGBUS, &sa, NULL);
	*/
}

// 
#define NONFAILING(...) {__VA_ARGS__;}

#endif
/*atomic_add(&skip_segv, 1) ;                                          \
if (_setjmp(segv_env) == 0) {                        \
	__VA_ARGS__;                                 \
}                                                    \
atomic_sub(&skip_segv, 1) ;      \*/

#if defined(SYZ_EXECUTOR) || (defined(SYZ_REPEAT) && defined(SYZ_WAIT_REPEAT))
static uint64 current_time_ms() // checked
{
	return gettime_msec() ;
}
#endif

#if defined(SYZ_EXECUTOR)
static void sleep_ms(uint64 ms) // checked
{
	sleep(ms);
}
#endif

#if defined(SYZ_EXECUTOR) || defined(SYZ_USE_TMP_DIR)
static void use_temporary_dir() // checked
{
	char tmpdir_template[] = "./syzkaller.maomao";
	int tmpdir = mkdir(tmpdir_template); // 原来是创建一个临时文件夹，现在也不需要了（安全？无所谓的）
	if (tmpdir == -1)
		fail("failed to mkdtemp");
	if (chdir(tmpdir_template))
		fail("failed to chdir");
}
#endif

#if defined(SYZ_EXECUTOR) || defined(SYZ_TUN_ENABLE)
static void vsnprintf_check(char* str, size_t size, const char* format, va_list args) // checked
{
	int rv;

	rv = vsnprintf(str, size, format, args);
	if (rv < 0)
		fail("tun: snprintf failed");
	if ((size_t)rv >= size)
		fail("tun: string '%s...' doesn't fit into buffer", str);
}

static void snprintf_check(char* str, size_t size, const char* format, ...) // checked
{
	va_list args;

	va_start(args, format);
	vsnprintf_check(str, size, format, args);
	va_end(args);
}

#define COMMAND_MAX_LEN 128
#define PATH_PREFIX "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin "
#define PATH_PREFIX_LEN (sizeof(PATH_PREFIX) - 1)

/*
static void execute_command(bool panic, const char* format, ...)
{
	va_list args;
	char command[PATH_PREFIX_LEN + COMMAND_MAX_LEN];
	int rv;

	va_start(args, format);
	// Executor process does not have any env, including PATH.
	// On some distributions, system/shell adds a minimal PATH, on some it does not.
	// Set own standard PATH to make it work across distributions.
	memcpy(command, PATH_PREFIX, PATH_PREFIX_LEN);
	vsnprintf_check(command + PATH_PREFIX_LEN, COMMAND_MAX_LEN, format, args);
	va_end(args);
	rv = system(command);
	if (rv) {
		if (panic)
			fail("command '%s' failed: %d", &command[0], rv);
		debug("command '%s': %d\n", &command[0], rv);
	}
}*/

static int tunfd = -1;
static int tun_frags_enabled;

//这一部分原本是很多个通过系统调用+回调函数实现的一些外部fuzzer功能，但是由于ucore不包含这些系统调用，因此这里全部删去了（实在是太长不好保留

#if defined(SYZ_EXECUTOR) || defined(SYZ_FAULT_INJECTION) || defined(SYZ_SANDBOX_NAMESPACE) || \
    defined(SYZ_ENABLE_CGROUPS)
static bool write_file(const char* file, const char* what, ...) // checked
{
	char buf[1024];
	va_list args;
	va_start(args, what);
	vsnprintf(buf, sizeof(buf), what, args);
	va_end(args);
	buf[sizeof(buf) - 1] = 0;
	int len = strlen(buf);

	int fd = open(file, O_WRONLY);
	if (fd == -1)
		return false;
	if (write(fd, buf, len) != len) {
		int err = errno;
		close(fd);
		errno = err;
		return false;
	}
	close(fd);
	return true;
}
#endif

// TODO(dvyukov): this should be under a separate define for separate minimization,
// but for now we bundle this with cgroups.
/*static void setup_binfmt_misc() // checked
{
	if (!write_file("/proc/sys/fs/binfmt_misc/register", ":syz0:M:0:syz0::./file0:")) {
		debug("write(/proc/sys/fs/binfmt_misc/register, syz0) failed: %d\n", errno);
	}
	if (!write_file("/proc/sys/fs/binfmt_misc/register", ":syz1:M:1:yz1::./file0:POC")) {
		debug("write(/proc/sys/fs/binfmt_misc/register, syz1) failed: %d\n", errno);
	}
}*/
#endif

#if defined(SYZ_EXECUTOR) || defined(SYZ_SANDBOX_NONE) || defined(SYZ_SANDBOX_SETUID) || defined(SYZ_SANDBOX_NAMESPACE)
static void loop();

static void sandbox_common() // checked
{
	//prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
	//setpgrp(); 设置进程组的操作，这个也不需要管了
	//setsid(); 设置会话的操作，更不用管

	// 设置进程占用资源的一些限制，checked
	struct linux_rlimit rlim;
	rlim.rlim_cur = rlim.rlim_max = 160 << 20;
	setrlimit(RLIMIT_AS, &rlim);
	rlim.rlim_cur = rlim.rlim_max = 8 << 20;
	setrlimit(RLIMIT_MEMLOCK, &rlim);
	rlim.rlim_cur = rlim.rlim_max = 136 << 20;
	setrlimit(RLIMIT_FSIZE, &rlim);
	rlim.rlim_cur = rlim.rlim_max = 1 << 20;
	setrlimit(RLIMIT_STACK, &rlim);
	rlim.rlim_cur = rlim.rlim_max = 0;
	setrlimit(RLIMIT_CORE, &rlim);

	// CLONE_NEWNS/NEWCGROUP cause EINVAL on some systems,
	// so we do them separately of clone in do_sandbox_namespace.
	/*if (unshare(CLONE_NEWNS)) {
		debug("unshare(CLONE_NEWNS): %d\n", errno);
	}
	if (unshare(CLONE_NEWIPC)) {
		debug("unshare(CLONE_NEWIPC): %d\n", errno);
	}
	if (unshare(0x02000000)) {
		debug("unshare(CLONE_NEWCGROUP): %d\n", errno);
	}
	if (unshare(CLONE_NEWUTS)) {
		debug("unshare(CLONE_NEWUTS): %d\n", errno);
	}
	if (unshare(CLONE_SYSVSEM)) {
		debug("unshare(CLONE_SYSVSEM): %d\n", errno);
	}*/
}
#endif

#if defined(SYZ_EXECUTOR) || defined(SYZ_SANDBOX_NONE)
static int do_sandbox_none(void)
{
	// CLONE_NEWPID takes effect for the first child of the current process,
	// so we do it before fork to make the loop "init" process of the namespace.
	// We ought to do fail here, but sandbox=none is used in pkg/ipc tests
	// and they are usually run under non-root.
	// Also since debug is stripped by pkg/csource, we need to do {}
	// even though we generally don't do {} around single statements.
	
	// ucore根本就没有这个操作，先删了试试
	/*if (unshare(CLONE_NEWPID)) {
		debug("unshare(CLONE_NEWPID): %d\n", errno); 
	}*/
	int pid = fork(); // 建子进程
	if (pid < 0)
		fail("sandbox fork failed");
	if (pid)
		return pid;

	//下面是子进程的操作
#if defined(SYZ_EXECUTOR) || defined(SYZ_ENABLE_CGROUPS)
	//setup_binfmt_misc(); //修改binfmt_misc，目前还不知道作用如何
#endif
	sandbox_common(); // 设置sandbox的一些初始设置
	/*if (unshare(CLONE_NEWNET)) {
		debug("unshare(CLONE_NEWNET): %d\n", errno);
	}*/

	loop();
	doexit(1);
}
#endif

// 先不处理一切和网络相关的部分，这一块删掉了很多函数


#if defined(SYZ_EXECUTOR) || (defined(SYZ_REPEAT) && defined(SYZ_WAIT_REPEAT) && defined(SYZ_USE_TMP_DIR))
// One does not simply remove a directory.
// There can be mounts, so we need to try to umount.
// Moreover, a mount can be mounted several times, so we need to try to umount in a loop.
// Moreover, after umount a dir can become non-empty again, so we need another loop.
// Moreover, a mount can be re-mounted as read-only and then we will fail to make a dir empty.
static void remove_dir(const char* dir) // checked...?
{
	DIR* dp;
	struct dirent* ep;
	int iter = 0;
retry:
	// 由于没有mount系统调用，所以暂时不需要umount
	//while (umount(dir) == 0) { 
	//	debug("umount(%s)\n", dir);
	//}
	dp = opendir(dir);
	if (dp == NULL) {
		exitf("opendir(%s) failed", dir);
	}
	while ((ep = readdir(dp))) {
		if (strcmp(ep->name, ".") == 0 || strcmp(ep->name, "..") == 0)
			continue;
		char filename[4096];
		snprintf(filename, sizeof(filename), "%s/%s", dir, ep->name);
		struct linux_stat st;
		if (lstat(filename, &st)) 
			exitf("lstat(%s) failed", filename);
		if (S_ISDIR(st.st_mode)) {
			remove_dir(filename);
			continue;
		}
		int i;
		// 删除文件
		for (i = 0;; i++) {
			debug("unlink(%s)\n", filename);
			if (unlink(filename) == 0)
				break;
			/*if (errno == EROFS) {
				debug("ignoring EROFS\n");
				break;
			}*/
			if (i > 100)
				exitf("unlink(%s) failed", filename);
			debug("umount(%s)\n", filename);
			//if (umount(filename))
			//	exitf("umount(%s) failed", filename);
		}
	}
	closedir(dp);
	int i;
	//need update UCORE居然没有rmdir操作？？？震惊了，先去掉这一步吧
	/*for (i = 0;; i++) {
		debug("rmdir(%s)\n", dir);
		if (rmdir(dir) == 0) 
			break;
		if (i < 100) {
			if (errno == EROFS) {
				debug("ignoring EROFS\n");
				break;
			}
			if (errno == EBUSY) {
				debug("umount(%s)\n", dir);
				if (umount(dir))
					exitf("umount(%s) failed", dir);
				continue;
			}
			if (errno == ENOTEMPTY) {
				if (iter < 100) {
					iter++;
					goto retry;
				}
			}
		}
		exitf("rmdir(%s) failed", dir);
	}*/
}
#endif

#if defined(SYZ_EXECUTOR) || defined(SYZ_FAULT_INJECTION)
static int inject_fault(int nth) // checked
{
	int fd;
	char buf[16];

	fd = open("/proc/thread-self/fail-nth", O_RDWR);
	// We treat errors here as temporal/non-critical because we see
	// occasional ENOENT/EACCES errors returned. It seems that fuzzer
	// somehow gets its hands to it.
	if (fd == -1)
		exitf("failed to open /proc/thread-self/fail-nth");
	snprintf(buf, 16, "%d", nth + 1);
	if (write(fd, buf, strlen(buf)) != strlen(buf))
		exitf("failed to write /proc/thread-self/fail-nth");
	return fd;
}
#endif

#if defined(SYZ_EXECUTOR)
static int fault_injected(int fail_fd) // checked
{
	char buf[16];
	int n = read(fail_fd, buf, sizeof(buf) - 1);
	if (n <= 0)
		exitf("failed to read /proc/thread-self/fail-nth");
	int res = n == 2 && buf[0] == '0' && buf[1] == '\n';
	buf[0] = '0';
	if (write(fail_fd, buf, 1) != 1)
		exitf("failed to write /proc/thread-self/fail-nth");
	close(fail_fd);
	return res;
}
#endif

#if defined(SYZ_EXECUTOR) || defined(SYZ_REPEAT)
static void execute_one();
extern unsigned long long procid;

#if defined(SYZ_EXECUTOR)
void reply_handshake();
void receive_execute(bool need_prog);
void reply_execute(int status);
extern sem_t output_data_mutex ;
extern uint32* output_data;
extern uint32* output_pos;
#endif

#if defined(SYZ_EXECUTOR) || defined(SYZ_WAIT_REPEAT)
static void loop()
{
#if defined(SYZ_EXECUTOR)
	// Tell parent that we are ready to serve.
	// 需要修改成qemu模式
	reply_handshake(); 
#endif
#if defined(SYZ_EXECUTOR) || defined(SYZ_RESET_NET_NAMESPACE)
	//checkpoint_net_namespace();
#endif

	int iter;
	//不断做测试
	for (iter = 0;; iter++) {
#if defined(SYZ_EXECUTOR) || defined(SYZ_USE_TMP_DIR)
		// Create a new private work dir for this test (removed at the end of the loop).
		char cwdbuf[32];
		snprintf(cwdbuf, 32, "./%d", iter);
		if (mkdir(cwdbuf))
			fail("failed to mkdir");
#endif
// 这一段没看懂，但应该是和external fuzzer相关，删了
#if defined(SYZ_EXECUTOR) || defined(__NR_syz_mount_image) || defined(__NR_syz_read_part_table)
		/*
		char buf[64];
		snprintf(buf, sizeof(buf), "/dev/loop%llu", procid);
		int loopfd = open(buf, O_RDWR);
		if (loopfd != -1) {
			ioctl(loopfd, LOOP_CLR_FD, 0);
			close(loopfd);
		}*/
#endif
#if defined(SYZ_EXECUTOR)
		// TODO: consider moving the read into the child.
		// Potentially it can speed up things a bit -- when the read finishes
		// we already have a forked worker process.
		debug("receiving execute signal\n") ;
		receive_execute(false);
#endif
		//建立子进程开始工作
		int pid = fork();
		if (pid < 0)
			fail("clone failed");
		if (pid == 0) { //子进程开始干活
			//prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
			//setpgrp();
#if defined(SYZ_EXECUTOR) || defined(SYZ_USE_TMP_DIR)
			if (chdir(cwdbuf))
				fail("failed to chdir");
#endif
#if defined(SYZ_EXECUTOR)
			// close(kInPipeFd); // 干活进程不再需要管两个Pipe
			// close(kOutPipeFd);
#endif
#if defined(SYZ_EXECUTOR)
			output_pos = output_data;
#endif
			//运行测试程序
			execute_one();
			debug("worker exiting\n");
			doexit(0);
		}
		debug("spawned worker pid %d\n", pid);

		//以下是父进程的操作
		// We used to use sigtimedwait(SIGCHLD) to wait for the subprocess.
		// But SIGCHLD is also delivered when a process stops/continues,
		// so it would require a loop with status analysis and timeout recalculation.
		// SIGCHLD should also unblock the usleep below, so the spin loop
		// should be as efficient as sigtimedwait.
		int status = 0;
		//记录开始时间
		uint64 start = current_time_ms();
#if defined(SYZ_EXECUTOR)
		//记录已经处理的calls
		uint64 last_executed = start;
		// 用semaphone代替了atomic_load
		debug("waiting for output_data_mutex...\n") ;
		sem_wait(output_data_mutex) ;
		uint32 executed_calls = *output_data;
		sem_post(output_data_mutex) ;
		debug("wait ended...\n") ;
#endif
		//等子进程结束
		for (;;) {
			int res = waitpid(-1, &status, 1); // 1 means WNOHANG
			debug("waiting for pid: %d\n", res) ;
			if (res == pid) {
				// 结束了
				debug("waitpid(%d)=%d\n", pid, res);
				break;
			}
			sleep(1);
#if defined(SYZ_EXECUTOR)
			// Even though the test process executes exit at the end
			// and execution time of each syscall is bounded by 20ms,
			// this backup watchdog is necessary and its performance is important.
			// The problem is that exit in the test processes can fail (sic).
			// One observed scenario is that the test processes prohibits
			// exit_group syscall using seccomp. Another observed scenario
			// is that the test processes setups a userfaultfd for itself,
			// then the main thread hangs when it wants to page in a page.
			// Below we check if the test process still executes syscalls
			// and kill it after 500ms of inactivity.
			uint64 now = current_time_ms();
			
			debug("current_time: %d...\n", now) ;
			// 用semaphone代替了atomic_load
			debug("waiting for output_data_mutex...\n") ;
			sem_wait(output_data_mutex) ;
			uint32 now_executed = *output_data;
			sem_post(output_data_mutex) ;
			debug("wait ended...\n") ;

			if (executed_calls != now_executed) {
				executed_calls = now_executed;
				last_executed = now;
			}
			if ((now - start < 3 * 1000) && (now - start < 1000 || now - last_executed < 500))
				continue;
#else
			if (current_time_ms() - start < 3 * 1000)
				continue;
#endif
			//过了一定时间就杀了工作进程
			debug("waitpid(%d)=%d\n", pid, res);
			debug("killing\n");
			//kill(-pid);
			kill(pid);
			while (waitpid(-1, &status, 0) != pid) {
			}
			break;
		}
#if defined(SYZ_EXECUTOR)
		status = WEXITSTATUS(status);
		if (status == kFailStatus)
			fail("child failed");
		if (status == kErrorStatus)
			error("child errored");
		reply_execute(0);
#endif
#if defined(SYZ_EXECUTOR) || defined(SYZ_USE_TMP_DIR)
		debug("removing dir\n") ;
		remove_dir(cwdbuf);
#endif
#if defined(SYZ_EXECUTOR) || defined(SYZ_RESET_NET_NAMESPACE)
		//reset_net_namespace();
#endif
	}
}
#endif
#endif

#if defined(SYZ_THREADED) // 删掉了多进程的部分
#endif
