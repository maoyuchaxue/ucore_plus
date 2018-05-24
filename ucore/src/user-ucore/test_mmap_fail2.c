/*SYS_pipe(&(0x7f0000000000)=<r0=>0xffffffffffffffff)
SYS_fsync(r0)
SYS_fsync(r0)
r1 = SYS_open(&(0x7f0000000040)='./file0\x00', 0x901)
SYS_seek(r1, 0x0, 0x200)
SYS_pipe(&(0x7f0000000080)=<r2=>0xffffffffffffffff)
SYS_linux_mmap(0x1, 0x5, 0x5, 0x200, r1, 0x0)
SYS_seek(r2, 0x9, 0x1)
SYS_linux_mmap(0x7, 0x3, 0x2, 0xf90586166d82955a, r1, 0x48)
*/
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
	
	sys_linux_mmap(0x7, 0x3, 0x2, 0xf90586166d82955a, 0xffffffffffffffff, 0x48) ;
	return 0;
}
