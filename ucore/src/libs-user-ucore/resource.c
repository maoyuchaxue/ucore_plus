#include <syscall.h>
#include "resource.h"

int setrlimit(int res, struct linux_rlimit * __limit) 
{
    return sys_linux_setrlimit(res, __limit) ;
}
int getrlimit(int res, struct linux_rlimit * __limit) 
{
    return sys_linux_getrlimit(res, __limit) ;
}