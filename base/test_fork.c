/*
 * A simple test program for fork function
 * fork: 返回0表示子进程, >0表示父进程，值为子进程id，子进程创建后获得父进程数据空间，
 * 堆，栈的副本，父子进程并不共享这些存储空间，父子进程共享正文段。
 */

#include <unistd.h>     /* for fork function */

#include <stdlib.h>     /* for exit function */
#include <stdio.h>      /* for perror function */    

static int global_var = 9;

int main()
{
    pid_t pid;
    int local_var = 9;

    if ((pid = fork()) < 0) 
    {
        perror("fork");
        exit(1);
    } 
    else if (pid == 0)  /* child process */
    {
        global_var++;
        local_var++;
    }
    else        /* parent process */
    {
        sleep(2);
    }

    printf("pid = %d, global_var = %d, local_var = %d\n", getpid(), global_var, local_var);

    return 0;
}

