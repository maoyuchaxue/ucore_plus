#include <ulib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <dir.h>
#include <file.h>
#include <error.h>
#include <unistd.h>
#include <syscall.h>


int main(int argc, char **argv)
{
    int ret;
    unsigned long long a, b;

    a = 0x3000000000;
	ret = sys_mmap(&a, 100, 0);
    cprintf("test 1, a: %lld, ret %d\n", a, ret);
    b = a + 1;
    ret = sys_mmap(&b, 98, 0);
    cprintf("test 1, b: %lld, ret %d\n", b, ret); // works well

    a = 0x4000000000;
	ret = sys_mmap(&a, 100, 0);
    cprintf("test 2, a: %lld, ret %d\n", a, ret);
    b = a - 1;
    ret = sys_mmap(&b, 101, 0);
    cprintf("test 2, b: %lld, ret %d\n", b, ret); // wouldd fail
	return 0;
}
