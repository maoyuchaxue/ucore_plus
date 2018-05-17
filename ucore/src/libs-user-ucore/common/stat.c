#include <stat.h>
#include <syscall.h>

int lstat(const char *path, struct linux_stat *linux_stat_store) 
{
    return sysfile_linux_lstat(path, linux_stat_store) ;
}