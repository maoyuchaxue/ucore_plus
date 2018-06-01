#include <ulib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <dir.h>
#include <file.h>
#include <error.h>
#include <unistd.h>
#include <syscall.h>


char *fn;
int main(int argc, char **argv)
{
    fn = shmem_malloc(4096);
    long long len = 0x80000001;
    // int r0 = sys_open("/file0", O_CREAT | O_WRONLY);
    // sys_write(r0, " ", 1);
    // sys_close(r0);

    int r1 = sys_open("file0", 0xf42);
    cprintf("%d\n", r1);
    long long r2 = sys_linux_mmap(0x7fffffff, 0x3, 0xc, 0x100, r1, 0x7);
    cprintf("0x%08x\n", r2);
    int r3 = sys_seek(r1, 0x9, len);
    cprintf("%d\n", r3);
    int r = sys_write(r1, fn, len);
    cprintf("%d\n", r);
    cprintf("finished\n"); // would fail
	return 0;
}
