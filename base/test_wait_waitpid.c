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
    /* �������� */
    if (WIFEXITED(status))
    {
        printf("normal termination, exit status = %d \r\n", WEXITSTATUS(status));
    } 
    /* �쳣��ֹ */
    else if (WIFSIGNALED(status))
    {
        printf("abnormal termination, signal number = %d%s\n", WTERMSIG(status), 
#ifdef WCOREDUMP
                WCOREDUMP(status) ? "(core file generated)" : "");
#else
                "");
#endif    
    }
    /* ��ͣ�ӽ���״̬ */
    else if (WIFSTOPPED(status))
    {
        printf("child stopped, signal number = %d\r\n", WSTOPSIG(status));
    }
}

int main()
{
    pid_t pid;
    int status;

    /* �������� */    
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

    /* abort�ź��쳣���� */
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

    /* ��0�쳣���� */
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
