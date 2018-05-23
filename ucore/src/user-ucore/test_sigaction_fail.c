#include <ulib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <dir.h>
#include <file.h>
#include <error.h>
#include <unistd.h>
#include <syscall.h>
#include <signal.h>
#include <signum.h>


int main(int argc, char **argv)
{
    // struct sigaction {
    //     sighandler_t sa_handler;
    //     void (*sa_sigaction) (int, struct siginfo_t *, void *);
    //     sigset_t sa_mask;
    //     int sa_flags;
    //     void (*sa_restorer) (void);
    // };

    char corrupt_data[20];
    int i = 0;
    for (; i < 20; i++) {
        corrupt_data[i] = 0xff;
    }
    sigaction(0x41, (struct sigaction *)(&corrupt_data[0]), 0);
    cprintf("finished\n"); // would fail
	return 0;
}
