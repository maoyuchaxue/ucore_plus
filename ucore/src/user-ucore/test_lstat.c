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

static const char *g_envp[] = { "PATH=/bin", NULL };

static char *shcwd = NULL;

__attribute__((noreturn)) static void doexit(int status) // checked
{
	volatile unsigned i;
	syscall(SYS_exit, status);
	for (i = 0;; i++) {
	}
}

int main(int argc, char **argv)
{
	int pid = fork() ;
	struct linux_stat st;
	if(pid < 0)
		return 1 ;
	else if(pid == 0)
	{
		open("./file0", O_CREAT) ;
		//printf("son: lstat result: %d\n", lstat("./cat", &st)) ;
		doexit(0) ;
	}
	
	int status = 0;
	while(waitpid(pid, &status, 0) != pid) ;
	printf("parent: lstat result: %d\n", lstat("./file0", &st)) ;
	return 0 ;
}
