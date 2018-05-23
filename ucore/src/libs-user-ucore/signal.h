#include <signum.h>

sighandler_t signal(int sign, sighandler_t handler);
int tkill(int pid, int sign);
int kill_bionic(int pid, int sign);
int sigprocmask(int how, const sigset_t * set, sigset_t * old);
int sigsuspend(uint32_t mask);
int sigaction(int sign, const struct sigaction *act, struct sigaction *old);
int set_shellrun_pid();

#define _NSIG         1024
#define _NSIG_BPW     (8 * sizeof (unsigned long int))
#define _NSIG_WORDS	  (_NSIG / _NSIG_BPW)