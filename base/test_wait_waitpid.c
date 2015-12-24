/*
 * A simple test program for wait and waitpid function
 * time: 2013-11-08 16:24
 */

#include <sys/wait.h>       /* wait,waitpid */
#include <unistd.h>
#include <stdio.h>

#include <stdlib.h>         /* abort */

void pri_exit(int status)
{
    /* 正常结束 */
    if (WIFEXITED(status))
    {
        printf("normal termination, exit status = %d \r\n", WEXITSTATUS(status));
    } 
    /* 异常终止 */
    else if (WIFSIGNALED(status))
    {
        printf("abnormal termination, signal number = %d%s\n", WTERMSIG(status), 
#ifdef WCOREDUMP
                WCOREDUMP(status) ? "(core file generated)" : "");
#else
                "");
#endif    
    }
    /* 暂停子进程状态 */
    else if (WIFSTOPPED(status))
    {
        printf("child stopped, signal number = %d\r\n", WSTOPSIG(status));
    }
}

int main()
{
    pid_t pid;
    int status;

    /* 正常结束 */    
    if ((pid = fork()) < 0)
    {
        perror("fork");
        exit(1);
    }
    else if (pid == 0)
    {
        exit(7);
    }

    if (wait(&status) != pid)
    {
        perror("wait");
        exit(1);
    }

    pri_exit(status);

    /* abort信号异常结束 */
    if ((pid = fork()) < 0)
    {
        perror("fork");
        exit(1);
    }
    else if (pid == 0)
    {
        abort();
    }

    if (wait(&status) != pid)
    {
        perror("wait");
        exit(1);
    }

    pri_exit(status);

    /* 除0异常结束 */
    if ((pid = fork()) < 0)
    {
        perror("fork");
        exit(1);
    }
    else if (pid == 0)
    {
        status /= 0;
    }

    if (wait(&status) != pid)
    {
        perror("wait");
        exit(1);
    }

    pri_exit(status);

    exit(0);
}

/*
 * result: 
 */
