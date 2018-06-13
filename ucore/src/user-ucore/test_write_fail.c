#include <ulib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <dir.h>
#include <file.h>
#include <stat.h>
#include <error.h>
#include <unistd.h>

#define printf(...)                     fprintf(1, __VA_ARGS__)
#define putc(c)                         printf("%c", c)

#define BUFSIZE                         4096
#define WHITESPACE                      " \t\r\n"
#define SYMBOLS                         "<|>&;"

char* buf ;
int main(int argc, char **argv)
{
    printf("%016x\n", &buf) ;

    sem_t r0 = sys_sem_init(0x2) ;
    sys_sem_get_value(r0, 0x5) ;
    sys_sem_post(r0) ;
    sys_sem_post(r0) ;
    int r1 = sys_open("./file0", 0x1800) ;
    sys_write(r1, buf+0x40, 0x0) ;
    sys_seek(r1, 0x7, 0xe0d) ;
    sys_chdir("./file0") ;
    int r2 = sys_open("./file0", 0x2880) ;
    sys_fsync(r1) ;
    sys_unlink("./file0") ;
    sys_linux_sigprocmask(0x2, buf+0x140, buf+0x180) ;
    sys_rename("./file0", "./file0") ;
    sys_seek(r2, 0xd5, 0x1) ;
    sys_getdirentry(r2, buf+240) ;
    sys_linux_sigprocmask(0x1, buf+0x380, buf+0x3c0) ;
    int r3 = sys_open("./file0", 0x28c2) ;
    printf("open fd: %d\n", r3) ;
    sys_sem_init(0x1f) ;
    int ret = sys_write(r3, buf+0x440, 0xffffffffffffff7f) ;
    printf("write finished: %d\n", ret) ;
	return 0 ;
}

/*
r0 = SYS_sem_init(0x2)
SYS_sem_get_value(r0, 0x5)
SYS_sem_post(r0)
SYS_sem_post(r0)
r1 = SYS_open(&(0x7f0000000000)='./file0\x00', 0x1800)
SYS_write(r1, &(0x7f0000000040), 0x0)
SYS_seek(r1, 0x7, 0xe0d)
SYS_chdir(&(0x7f0000000080)='./file0\x00')
r2 = SYS_open(&(0x7f00000000c0)='./file0\x00', 0x2880)
SYS_fsync(r1)
SYS_unlink(&(0x7f0000000100)='./file0\x00')
SYS_linux_sigprocmask(0x2, &(0x7f0000000140)={[0x0, 0x4, 0xfffffffffffffffc, 0x0, 0x3, 0xff, 0x3, 0x8001, 0x1, 0x0, 0x3f, 0x1, 0xf3, 0x6, 0x0, 0x2]}, &(0x7f0000000180))
SYS_pgdir()
SYS_rename(&(0x7f00000001c0)='./file0\x00', &(0x7f0000000200)='./file0\x00')
SYS_seek(r2, 0xd5, 0x1)
SYS_getdirentry(r2, &(0x7f0000000240))
SYS_linux_sigprocmask(0x1, &(0x7f0000000380)={[0xffffffffffff7fff, 0x100, 0xd5a, 0x1ff, 0x400, 0x4, 0x2, 0x4, 0x65, 0x7, 0x43, 0x101, 0x3, 0x10000, 0x9, 0x4]}, &(0x7f00000003c0))
r3 = SYS_open(&(0x7f0000000400)='./file0\x00', 0x28c2)
SYS_sem_init(0x1f)
SYS_write(r3, &(0x7f0000000440), 0xffffffffffffff7f)
*/