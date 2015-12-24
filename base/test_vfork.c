/*
 * A simple test program for vfork function
 * time: 2013-11-08 14:29
 * fork vs vfork:
 * (1).the fork function create a new progress, and exec,vfork和fork都创建一个子进程，
 * 但vfork并不将父进程的地址空间完全复制到子进程中，因为子进程会立即调用exec或者exit,
 * 于是也就不会存访父进程的地址空间.在子进程调用exec或exit之前，它在父进程的空间中运行。
 * (2). vfork保证子进程先运行，在它调用exec或exit之后父进程才可能被调度运行。
 */

#include <sys/types.h>
#include <unistd.h>         // for exit()

#include <stdlib.h>         // for _exit()

#include <stdio.h>

int global_var = 6;

int main()
{
    int local_var = 7;
    pid_t pid;

    printf("before vfork \r\n");
    if ((pid = vfork()) < 0)
    {
        perror("vfork");
        exit(1);
    }
    else if (pid == 0)      /* child process */
    {
        global_var++;
        local_var++;
        _exit(0);
    }

    /* parent process */
    printf("pid = %d, global_var = %d, var = %d\n", getpid(), global_var, local_var);
    exit(0);
}

/*
 * result: pid = 15988, global_var = 7, var = 8
 *
 */

