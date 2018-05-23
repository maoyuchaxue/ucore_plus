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
    
	int fd1 = open("file1", O_CREAT | O_RDWR);
    int fd2 = -1;

    cprintf("before dup\n"); // should not fail

    sys_dup(fd1, fd2);

    cprintf("finished\n"); // would fail
	return 0;
}
